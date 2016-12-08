/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* Cairo - a vector graphics library with display and print output
 *
 * Copyright ?2010 Mozilla Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is the Mozilla Foundation
 *
 * Contributor(s):
 *	Bas Schouten <bschouten@mozilla.com>
 */

#include "cairoint.h"

#include "cairo-win32-private.h"
#include "cairo-surface-private.h"
#include "cairo-clip-private.h"

#include "cairo-dwrite-private.h"
#include "cairo-image-surface-private.h"
#include "cairo-pattern-private.h"
#include "cairo-surface-private.h"
#include "cairo-truetype-subset-private.h"
#include <float.h>
#include <Dwrite.h>

#define CAIRO_INT_STATUS_SUCCESS (cairo_int_status_t)CAIRO_STATUS_SUCCESS

// Forward declarations

cairo_int_status_t
_dwrite_draw_glyphs_to_gdi_surface_gdi(cairo_win32_surface_t *surface,
				       DWRITE_MATRIX *transform,
				       DWRITE_GLYPH_RUN *run,
				       COLORREF color,
				       cairo_dwrite_scaled_font_t *scaled_font,
				       const RECT &area);

IDWriteFactory *DWriteFactory::mFactoryInstance = NULL;
IDWriteFontCollection *DWriteFactory::mSystemCollection = NULL;
IDWriteRenderingParams *DWriteFactory::mDefaultRenderingParams = NULL;
IDWriteRenderingParams *DWriteFactory::mCustomClearTypeRenderingParams = NULL;
IDWriteRenderingParams *DWriteFactory::mForceGDIClassicRenderingParams = NULL;
FLOAT DWriteFactory::mGamma = -1.0;
FLOAT DWriteFactory::mEnhancedContrast = -1.0;
FLOAT DWriteFactory::mClearTypeLevel = -1.0;
int DWriteFactory::mPixelGeometry = -1;
int DWriteFactory::mRenderingMode = -1;

/* Functions cairo_font_face_backend_t */
static cairo_status_t
_cairo_dwrite_font_face_create_for_toy (cairo_toy_font_face_t   *toy_face,
					cairo_font_face_t      **font_face);
static cairo_bool_t
_cairo_dwrite_font_face_destroy (void *font_face);

static cairo_status_t
_cairo_dwrite_font_face_scaled_font_create (void			*abstract_face,
					    const cairo_matrix_t	*font_matrix,
					    const cairo_matrix_t	*ctm,
					    const cairo_font_options_t *options,
					    cairo_scaled_font_t **font);

const cairo_font_face_backend_t _cairo_dwrite_font_face_backend = {
    CAIRO_FONT_TYPE_DWRITE,
    _cairo_dwrite_font_face_create_for_toy,
    _cairo_dwrite_font_face_destroy,
    _cairo_dwrite_font_face_scaled_font_create
};

/* Functions cairo_scaled_font_backend_t */

void _cairo_dwrite_scaled_font_fini(void *scaled_font);

static cairo_warn cairo_int_status_t
_cairo_dwrite_scaled_glyph_init(void			     *scaled_font,
				cairo_scaled_glyph_t	     *scaled_glyph,
				cairo_scaled_glyph_info_t    info);

cairo_int_status_t
_cairo_dwrite_load_truetype_table(void		       *scaled_font,
				  unsigned long         tag,
				  long                  offset,
				  unsigned char        *buffer,
				  unsigned long        *length);

unsigned long
_cairo_dwrite_ucs4_to_index(void			     *scaled_font,
			    uint32_t			ucs4);

const cairo_scaled_font_backend_t _cairo_dwrite_scaled_font_backend = {
    CAIRO_FONT_TYPE_DWRITE,
    _cairo_dwrite_scaled_font_fini,
    _cairo_dwrite_scaled_glyph_init,
    NULL,
    _cairo_dwrite_ucs4_to_index,
    _cairo_dwrite_load_truetype_table,
    NULL,
};

/* Helper conversion functions */

/**
 * Get a DirectWrite matrix from a cairo matrix. Note that DirectWrite uses row
 * vectors where cairo uses column vectors. Hence the transposition.
 *
 * \param Cairo matrix
 * \return DirectWrite matrix
 */
DWRITE_MATRIX
_cairo_dwrite_matrix_from_matrix(const cairo_matrix_t *matrix)
{
    DWRITE_MATRIX dwmat;
    dwmat.m11 = (FLOAT)matrix->xx;
    dwmat.m12 = (FLOAT)matrix->yx;
    dwmat.m21 = (FLOAT)matrix->xy;
    dwmat.m22 = (FLOAT)matrix->yy;
    dwmat.dx = (FLOAT)matrix->x0;
    dwmat.dy = (FLOAT)matrix->y0;
    return dwmat;
}

/* Helper functions for cairo_dwrite_scaled_glyph_init */
cairo_int_status_t 
_cairo_dwrite_scaled_font_init_glyph_metrics 
    (cairo_dwrite_scaled_font_t *scaled_font, cairo_scaled_glyph_t *scaled_glyph);

cairo_int_status_t 
_cairo_dwrite_scaled_font_init_glyph_surface
    (cairo_dwrite_scaled_font_t *scaled_font, cairo_scaled_glyph_t *scaled_glyph);

cairo_int_status_t 
_cairo_dwrite_scaled_font_init_glyph_path
    (cairo_dwrite_scaled_font_t *scaled_font, cairo_scaled_glyph_t *scaled_glyph);

/* implement the font backend interface */

static cairo_status_t
_cairo_dwrite_font_face_create_for_toy (cairo_toy_font_face_t   *toy_face,
					cairo_font_face_t      **font_face)
{
    WCHAR *face_name;
    int face_name_len;

    if (!DWriteFactory::Instance()) {
	return (cairo_status_t)CAIRO_INT_STATUS_UNSUPPORTED;
    }

    face_name_len = MultiByteToWideChar(CP_UTF8, 0, toy_face->family, -1, NULL, 0);
    face_name = new WCHAR[face_name_len];
    MultiByteToWideChar(CP_UTF8, 0, toy_face->family, -1, face_name, face_name_len);

    IDWriteFontFamily *family = DWriteFactory::FindSystemFontFamily(face_name);
    delete face_name;
    if (!family) {
	*font_face = (cairo_font_face_t*)&_cairo_font_face_nil;
	return CAIRO_STATUS_FONT_TYPE_MISMATCH;
    }

    DWRITE_FONT_WEIGHT weight;
    switch (toy_face->weight) {
    case CAIRO_FONT_WEIGHT_BOLD:
	weight = DWRITE_FONT_WEIGHT_BOLD;
	break;
    case CAIRO_FONT_WEIGHT_NORMAL:
    default:
	weight = DWRITE_FONT_WEIGHT_NORMAL;
	break;
    }

    DWRITE_FONT_STYLE style;
    switch (toy_face->slant) {
    case CAIRO_FONT_SLANT_ITALIC:
	style = DWRITE_FONT_STYLE_ITALIC;
	break;
    case CAIRO_FONT_SLANT_OBLIQUE:
	style = DWRITE_FONT_STYLE_OBLIQUE;
	break;
    case CAIRO_FONT_SLANT_NORMAL:
    default:
	style = DWRITE_FONT_STYLE_NORMAL;
	break;
    }

    cairo_dwrite_font_face_t *face = (cairo_dwrite_font_face_t*)malloc(sizeof(cairo_dwrite_font_face_t));
    HRESULT hr = family->GetFirstMatchingFont(weight, DWRITE_FONT_STRETCH_NORMAL, style, &face->font);
    if (SUCCEEDED(hr)) {
	// Cannot use C++ style new since cairo deallocates this.
	*font_face = (cairo_font_face_t*)face;
	_cairo_font_face_init (&(*(_cairo_dwrite_font_face**)font_face)->base, &_cairo_dwrite_font_face_backend);
    } else {
	free(face);
    }

    return CAIRO_STATUS_SUCCESS;
}

static cairo_bool_t
_cairo_dwrite_font_face_destroy (void *font_face)
{
    cairo_dwrite_font_face_t *dwrite_font_face = static_cast<cairo_dwrite_font_face_t*>(font_face);
    if (dwrite_font_face->dwriteface)
	dwrite_font_face->dwriteface->Release();
    if (dwrite_font_face->font)
	dwrite_font_face->font->Release();
    return true;
}


static inline unsigned short
read_short(const char *buf)
{
    return be16_to_cpu(*(unsigned short*)buf);
}

#define GASP_TAG 0x70736167
#define GASP_DOGRAY 0x2

static cairo_bool_t
do_grayscale(IDWriteFontFace *dwface, unsigned int ppem)
{
    void *tableContext;
    char *tableData;
    UINT32 tableSize;
    BOOL exists;
    dwface->TryGetFontTable(GASP_TAG, (const void**)&tableData, &tableSize, &tableContext, &exists);

    if (exists) {
	if (tableSize < 4) {
	    dwface->ReleaseFontTable(tableContext);
	    return true;
	}
	struct gaspRange {
	    unsigned short maxPPEM; // Stored big-endian
	    unsigned short behavior; // Stored big-endian
	};
	unsigned short numRanges = read_short(tableData + 2);
	if (tableSize < (UINT)4 + numRanges * 4) {
	    dwface->ReleaseFontTable(tableContext);
	    return true;
	}
	gaspRange *ranges = (gaspRange *)(tableData + 4);
	for (int i = 0; i < numRanges; i++) {
	    if (be16_to_cpu(ranges[i].maxPPEM) > ppem) {
		if (!(be16_to_cpu(ranges[i].behavior) & GASP_DOGRAY)) {
		    dwface->ReleaseFontTable(tableContext);
		    return false;
		}
		break;
	    }
	}
	dwface->ReleaseFontTable(tableContext);
    }
    return true;
}

/* Helper function also stolen from cairo-win32-font.c */

/**
* _cairo_win32_print_gdi_error:
* @context: context string to display along with the error
*
* Helper function to dump out a human readable form of the
* current error code.
*
* Return value: A cairo status code for the error code
**/
cairo_status_t
_cairo_win32_print_gdi_error(const char *context)
{
    void *lpMsgBuf;
    DWORD last_error = GetLastError();

    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        last_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf,
        0, NULL)) {
        fprintf(stderr, "%s: Unknown GDI error", context);
    }
    else {
        fprintf(stderr, "%s: %S", context, (wchar_t *)lpMsgBuf);

        LocalFree(lpMsgBuf);
    }

    fflush(stderr);

    /* We should switch off of last_status, but we'd either return
    * CAIRO_STATUS_NO_MEMORY or CAIRO_STATUS_UNKNOWN_ERROR and there
    * is no CAIRO_STATUS_UNKNOWN_ERROR.
    */
    return _cairo_error(CAIRO_STATUS_NO_MEMORY);
}

static cairo_bool_t
_have_cleartype_quality(void)
{
    OSVERSIONINFO version_info;

    version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&version_info)) {
        _cairo_win32_print_gdi_error("_have_cleartype_quality");
        return FALSE;
    }

    return (version_info.dwMajorVersion > 5 ||
        (version_info.dwMajorVersion == 5 &&
        version_info.dwMinorVersion >= 1));	/* XP or newer */
}

static BYTE
cairo_win32_get_system_text_quality(void)
{
    BOOL font_smoothing;
    UINT smoothing_type;

    if (!SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &font_smoothing, 0)) {
        _cairo_win32_print_gdi_error("_get_system_quality");
        return DEFAULT_QUALITY;
    }

    if (font_smoothing) {
        if (_have_cleartype_quality()) {
            if (!SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE,
                0, &smoothing_type, 0)) {
                _cairo_win32_print_gdi_error("_get_system_quality");
                return DEFAULT_QUALITY;
            }

            if (smoothing_type == FE_FONTSMOOTHINGCLEARTYPE)
                return CLEARTYPE_QUALITY;
        }

        return ANTIALIASED_QUALITY;
    }
    else {
        return DEFAULT_QUALITY;
    }
}

static cairo_status_t
_cairo_dwrite_font_face_scaled_font_create (void			*abstract_face,
					    const cairo_matrix_t	*font_matrix,
					    const cairo_matrix_t	*ctm,
					    const cairo_font_options_t  *options,
					    cairo_scaled_font_t **font)
{
    cairo_dwrite_font_face_t *font_face = static_cast<cairo_dwrite_font_face_t*>(abstract_face);

    // Must do malloc and not C++ new, since Cairo frees this.
    cairo_dwrite_scaled_font_t *dwriteFont = (cairo_dwrite_scaled_font_t*)malloc(sizeof(cairo_dwrite_scaled_font_t));
    *font = reinterpret_cast<cairo_scaled_font_t*>(dwriteFont);
    _cairo_scaled_font_init(&dwriteFont->base, &font_face->base, font_matrix, ctm, options, &_cairo_dwrite_scaled_font_backend);

    cairo_font_extents_t extents;

    DWRITE_FONT_METRICS metrics;
    font_face->dwriteface->GetMetrics(&metrics);

    extents.ascent = (FLOAT)metrics.ascent / metrics.designUnitsPerEm;
    extents.descent = (FLOAT)metrics.descent / metrics.designUnitsPerEm;
    extents.height = (FLOAT)(metrics.ascent + metrics.descent + metrics.lineGap) / metrics.designUnitsPerEm;
    extents.max_x_advance = 14.0;
    extents.max_y_advance = 0.0;

    dwriteFont->mat = dwriteFont->base.ctm;
    cairo_matrix_multiply(&dwriteFont->mat, &dwriteFont->mat, font_matrix);
    dwriteFont->mat_inverse = dwriteFont->mat;
    cairo_matrix_invert (&dwriteFont->mat_inverse);

    cairo_antialias_t default_quality = CAIRO_ANTIALIAS_SUBPIXEL;

    dwriteFont->measuring_mode = DWRITE_MEASURING_MODE_NATURAL;

    // The following code detects the system quality at scaled_font creation time,
    // this means that if cleartype settings are changed but the scaled_fonts
    // are re-used, they might not adhere to the new system setting until re-
    // creation.
    switch (cairo_win32_get_system_text_quality()) {
	case CLEARTYPE_QUALITY:
	    default_quality = CAIRO_ANTIALIAS_SUBPIXEL;
	    break;
	case ANTIALIASED_QUALITY:
	    default_quality = CAIRO_ANTIALIAS_GRAY;
	    dwriteFont->measuring_mode = DWRITE_MEASURING_MODE_GDI_CLASSIC;
	    break;
	case DEFAULT_QUALITY:
	    // _get_system_quality() seems to think aliased is default!
	    default_quality = CAIRO_ANTIALIAS_NONE;
	    dwriteFont->measuring_mode = DWRITE_MEASURING_MODE_GDI_CLASSIC;
	    break;
    }

    if (default_quality == CAIRO_ANTIALIAS_GRAY) {
	if (!do_grayscale(font_face->dwriteface, (unsigned int)_cairo_round(font_matrix->yy))) {
	    default_quality = CAIRO_ANTIALIAS_NONE;
	}
    }

    if (options->antialias == CAIRO_ANTIALIAS_DEFAULT) {
	dwriteFont->antialias_mode = default_quality;
    } else {
	dwriteFont->antialias_mode = options->antialias;
    }

    dwriteFont->manual_show_glyphs_allowed = TRUE;
    dwriteFont->rendering_mode =
        default_quality == CAIRO_ANTIALIAS_SUBPIXEL ?
            cairo_dwrite_scaled_font_t::TEXT_RENDERING_NORMAL : cairo_dwrite_scaled_font_t::TEXT_RENDERING_NO_CLEARTYPE;

    return _cairo_scaled_font_set_metrics (*font, &extents);
}

/* Implementation cairo_dwrite_scaled_font_backend_t */
void
_cairo_dwrite_scaled_font_fini(void *scaled_font)
{
}

static cairo_int_status_t
_cairo_dwrite_scaled_glyph_init(void			     *scaled_font,
				cairo_scaled_glyph_t	     *scaled_glyph,
				cairo_scaled_glyph_info_t    info)
{
    cairo_dwrite_scaled_font_t *scaled_dwrite_font = static_cast<cairo_dwrite_scaled_font_t*>(scaled_font);
    cairo_int_status_t status;

    if ((info & CAIRO_SCALED_GLYPH_INFO_METRICS) != 0) {
	status = _cairo_dwrite_scaled_font_init_glyph_metrics (scaled_dwrite_font, scaled_glyph);
	if (status)
	    return status;
    }

    if (info & CAIRO_SCALED_GLYPH_INFO_SURFACE) {
	status = _cairo_dwrite_scaled_font_init_glyph_surface (scaled_dwrite_font, scaled_glyph);
	if (status)
	    return status;
    }

    if ((info & CAIRO_SCALED_GLYPH_INFO_PATH) != 0) {
	status = _cairo_dwrite_scaled_font_init_glyph_path (scaled_dwrite_font, scaled_glyph);
	if (status)
	    return status;
    }

    return CAIRO_INT_STATUS_SUCCESS;
}

unsigned long
_cairo_dwrite_ucs4_to_index(void			     *scaled_font,
			    uint32_t		      ucs4)
{
    cairo_dwrite_scaled_font_t *dwritesf = static_cast<cairo_dwrite_scaled_font_t*>(scaled_font);
    cairo_dwrite_font_face_t *face = reinterpret_cast<cairo_dwrite_font_face_t*>(dwritesf->base.font_face);

    UINT16 index;
    face->dwriteface->GetGlyphIndices(&ucs4, 1, &index);
    return index;
}

/* cairo_dwrite_scaled_glyph_init helper function bodies */
cairo_int_status_t 
_cairo_dwrite_scaled_font_init_glyph_metrics(cairo_dwrite_scaled_font_t *scaled_font, 
					     cairo_scaled_glyph_t *scaled_glyph)
{
    UINT16 charIndex = (UINT16)_cairo_scaled_glyph_index (scaled_glyph);
    cairo_dwrite_font_face_t *font_face = (cairo_dwrite_font_face_t*)scaled_font->base.font_face;
    cairo_text_extents_t extents;

    DWRITE_GLYPH_METRICS metrics;
    DWRITE_FONT_METRICS fontMetrics;
    font_face->dwriteface->GetMetrics(&fontMetrics);
    HRESULT hr = font_face->dwriteface->GetDesignGlyphMetrics(&charIndex, 1, &metrics);
    if (FAILED(hr)) {
	return CAIRO_INT_STATUS_UNSUPPORTED;
    }

    // TODO: Treat swap_xy.
    extents.width = (FLOAT)(metrics.advanceWidth - metrics.leftSideBearing - metrics.rightSideBearing) /
	fontMetrics.designUnitsPerEm;
    extents.height = (FLOAT)(metrics.advanceHeight - metrics.topSideBearing - metrics.bottomSideBearing) /
	fontMetrics.designUnitsPerEm;
    extents.x_advance = (FLOAT)metrics.advanceWidth / fontMetrics.designUnitsPerEm;
    extents.x_bearing = (FLOAT)metrics.leftSideBearing / fontMetrics.designUnitsPerEm;
    extents.y_advance = 0.0;
    extents.y_bearing = (FLOAT)(metrics.topSideBearing - metrics.verticalOriginY) /
	fontMetrics.designUnitsPerEm;

    // We pad the extents here because GetDesignGlyphMetrics returns "ideal" metrics
    // for the glyph outline, without accounting for hinting/gridfitting/antialiasing,
    // and therefore it does not always cover all pixels that will actually be touched.
    if (scaled_font->base.options.antialias != CAIRO_ANTIALIAS_NONE &&
	extents.width > 0 && extents.height > 0) {
    

    // Original Mozilla codes are commented out. 
    // Without more padding, some glyphs of Korean fonts(e.g., dotum 20pt) are clipped at edges.
	//extents.width += scaled_font->mat_inverse.xx * 2;
	//extents.x_bearing -= scaled_font->mat_inverse.xx;
    extents.width += scaled_font->mat_inverse.xx * 4;
    extents.x_bearing -= scaled_font->mat_inverse.xx * 2;
    extents.height += scaled_font->mat_inverse.yy * 4;
    extents.y_bearing -= scaled_font->mat_inverse.yy * 2;
    }

    _cairo_scaled_glyph_set_metrics (scaled_glyph,
				     &scaled_font->base,
				     &extents);
    return CAIRO_INT_STATUS_SUCCESS;
}

/**
 * Stack-based helper implementing IDWriteGeometrySink.
 * Used to determine the path of the glyphs.
 */

class GeometryRecorder : public IDWriteGeometrySink
{
public:
    GeometryRecorder(cairo_path_fixed_t *aCairoPath) 
	: mCairoPath(aCairoPath) {}

    // IUnknown interface
    IFACEMETHOD(QueryInterface)(IID const& iid, OUT void** ppObject)
    {
	if (iid != __uuidof(IDWriteGeometrySink))
	    return E_NOINTERFACE;

	*ppObject = static_cast<IDWriteGeometrySink*>(this);

	return S_OK;
    }

    IFACEMETHOD_(ULONG, AddRef)()
    {
	return 1;
    }

    IFACEMETHOD_(ULONG, Release)()
    {
	return 1;
    }

    IFACEMETHODIMP_(void) SetFillMode(D2D1_FILL_MODE fillMode)
    {
	return;
    }

    STDMETHODIMP Close()
    {
	return S_OK;
    }

    IFACEMETHODIMP_(void) SetSegmentFlags(D2D1_PATH_SEGMENT vertexFlags)
    {
	return;
    }
    
    cairo_fixed_t GetFixedX(const D2D1_POINT_2F &point)
    {
	unsigned int control_word;
	_controlfp_s(&control_word, _CW_DEFAULT, MCW_PC);
	return _cairo_fixed_from_double(point.x);
    }

    cairo_fixed_t GetFixedY(const D2D1_POINT_2F &point)
    {
	unsigned int control_word;
	_controlfp_s(&control_word, _CW_DEFAULT, MCW_PC);
	return _cairo_fixed_from_double(point.y);
    }

    IFACEMETHODIMP_(void) BeginFigure(
	D2D1_POINT_2F startPoint, 
	D2D1_FIGURE_BEGIN figureBegin) 
    {
	mStartPoint = startPoint;
	cairo_status_t status = _cairo_path_fixed_move_to(mCairoPath, 
							  GetFixedX(startPoint),
							  GetFixedY(startPoint));
    }

    IFACEMETHODIMP_(void) EndFigure(    
	D2D1_FIGURE_END figureEnd) 
    {
	if (figureEnd == D2D1_FIGURE_END_CLOSED) {
	    cairo_status_t status = _cairo_path_fixed_line_to(mCairoPath,
							      GetFixedX(mStartPoint), 
							      GetFixedY(mStartPoint));
	}
    }

    IFACEMETHODIMP_(void) AddBeziers(
	const D2D1_BEZIER_SEGMENT *beziers,
	UINT beziersCount)
    {
	for (unsigned int i = 0; i < beziersCount; i++) {
	    cairo_status_t status = _cairo_path_fixed_curve_to(mCairoPath,
							       GetFixedX(beziers[i].point1),
							       GetFixedY(beziers[i].point1),
							       GetFixedX(beziers[i].point2),
							       GetFixedY(beziers[i].point2),
							       GetFixedX(beziers[i].point3),
							       GetFixedY(beziers[i].point3));
	}	
    }

    IFACEMETHODIMP_(void) AddLines(
	const D2D1_POINT_2F *points,
	UINT pointsCount)
    {
	for (unsigned int i = 0; i < pointsCount; i++) {
	    cairo_status_t status = _cairo_path_fixed_line_to(mCairoPath, 
		GetFixedX(points[i]), 
		GetFixedY(points[i]));
	}
    }

private:
    cairo_path_fixed_t *mCairoPath;
    D2D1_POINT_2F mStartPoint;
};

cairo_int_status_t 
_cairo_dwrite_scaled_font_init_glyph_path(cairo_dwrite_scaled_font_t *scaled_font, 
					  cairo_scaled_glyph_t *scaled_glyph)
{
    cairo_path_fixed_t *path;
    path = _cairo_path_fixed_create();
    GeometryRecorder recorder(path);

    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0;
    offset.ascenderOffset = 0;
    UINT16 glyphId = (UINT16)_cairo_scaled_glyph_index(scaled_glyph);
    FLOAT advance = 0.0;
    cairo_dwrite_font_face_t *dwriteff = (cairo_dwrite_font_face_t*)scaled_font->base.font_face;
    dwriteff->dwriteface->GetGlyphRunOutline((FLOAT)scaled_font->base.font_matrix.yy,
					     &glyphId,
					     &advance,
					     &offset,
					     1,
					     FALSE,
					     FALSE,
					     &recorder);
    _cairo_path_fixed_close_path(path);

    /* Now apply our transformation to the drawn path. */
    _cairo_path_fixed_transform(path, &scaled_font->base.ctm);
    
    _cairo_scaled_glyph_set_path (scaled_glyph,
				  &scaled_font->base,
				  path);
    return CAIRO_INT_STATUS_SUCCESS;
}

/* Helper function also stolen from cairo-win32-font.c */

static cairo_surface_t *
_compute_mask (cairo_surface_t *surface,
	       int quality)
{
    cairo_image_surface_t *glyph;
    cairo_image_surface_t *mask;
    int i, j;

    glyph = (cairo_image_surface_t *)cairo_surface_map_to_image (surface, NULL);
    if (unlikely (glyph->base.status))
	return &glyph->base;

    if (quality == CLEARTYPE_QUALITY) {
	/* Duplicate the green channel of a 4-channel mask into the
	 * alpha channel, then invert the whole mask.
	 */
	mask = (cairo_image_surface_t *)
	    cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					glyph->width, glyph->height);
	if (likely (mask->base.status == CAIRO_STATUS_SUCCESS)) {
	    for (i = 0; i < glyph->height; i++) {
		uint32_t *p = (uint32_t *) (glyph->data + i * glyph->stride);
		uint32_t *q = (uint32_t *) (mask->data + i * mask->stride);

		for (j = 0; j < glyph->width; j++) {
		    *q++ = 0xffffffff ^ (*p | ((*p & 0x0000ff00) << 16));
		    p++;
		}
	    }
        mask->base.is_clear = 0;
	}
    } else {
	/* Compute an alpha-mask from a using the green channel of a
	 * (presumed monochrome) RGB24 image.
	 */
	mask = (cairo_image_surface_t *)
	    cairo_image_surface_create (CAIRO_FORMAT_A8,
					glyph->width, glyph->height);
	if (likely (mask->base.status == CAIRO_STATUS_SUCCESS)) {
	    for (i = 0; i < glyph->height; i++) {
		uint32_t *p = (uint32_t *) (glyph->data + i * glyph->stride);
		uint8_t *q = (uint8_t *) (mask->data + i * mask->stride);

		for (j = 0; j < glyph->width; j++)
		    *q++ = 255 - ((*p++ & 0x0000ff00) >> 8);
	    }
        mask->base.is_clear = 0;
	}
    }

    cairo_surface_unmap_image (surface, &glyph->base);
    return &mask->base;
}

cairo_int_status_t 
_cairo_dwrite_scaled_font_init_glyph_surface(cairo_dwrite_scaled_font_t *scaled_font, 
					     cairo_scaled_glyph_t	*scaled_glyph)
{
    cairo_int_status_t status;
    cairo_glyph_t glyph;
    cairo_win32_surface_t *surface;
    cairo_t *cr;
    cairo_surface_t *image;
    int width, height;
    double x1, y1, x2, y2;

    x1 = _cairo_fixed_integer_floor (scaled_glyph->bbox.p1.x);
    y1 = _cairo_fixed_integer_floor (scaled_glyph->bbox.p1.y);
    x2 = _cairo_fixed_integer_ceil (scaled_glyph->bbox.p2.x);
    y2 = _cairo_fixed_integer_ceil (scaled_glyph->bbox.p2.y);
    width = (int)(x2 - x1);
    height = (int)(y2 - y1);

    glyph.index = _cairo_scaled_glyph_index (scaled_glyph);
    glyph.x = -x1;
    glyph.y = -y1;

    DWRITE_GLYPH_RUN run;
    FLOAT advance = 0;
    UINT16 index = (UINT16)glyph.index;
    DWRITE_GLYPH_OFFSET offset;
    double x = glyph.x;
    double y = glyph.y;
    RECT area;
    DWRITE_MATRIX matrix;

    surface = (cairo_win32_surface_t *)
	cairo_win32_surface_create_with_dib (CAIRO_FORMAT_RGB24, width, height);

    cr = cairo_create (&surface->base);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    status = (cairo_int_status_t)cairo_status (cr);
    cairo_destroy(cr);
    if (status)
	goto FAIL;

    /**
     * We transform by the inverse transformation here. This will put our glyph
     * locations in the space in which we draw. Which is later transformed by
     * the transformation matrix that we use. This will transform the
     * glyph positions back to where they were before when drawing, but the
     * glyph shapes will be transformed by the transformation matrix.
     */
    cairo_matrix_transform_point(&scaled_font->mat_inverse, &x, &y);
    offset.advanceOffset = (FLOAT)x;
    /** Y-axis is inverted */
    offset.ascenderOffset = -(FLOAT)y;

    area.top = 0;
    area.bottom = height;
    area.left = 0;
    area.right = width;

    run.glyphCount = 1;
    run.glyphAdvances = &advance;
    run.fontFace = ((cairo_dwrite_font_face_t*)scaled_font->base.font_face)->dwriteface;
    run.fontEmSize = 1.0f;
    run.bidiLevel = 0;
    run.glyphIndices = &index;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    matrix = _cairo_dwrite_matrix_from_matrix(&scaled_font->mat);

    status = _dwrite_draw_glyphs_to_gdi_surface_gdi (surface, &matrix, &run,
            RGB(0,0,0), scaled_font, area);
    if (status)
	goto FAIL;

    GdiFlush();

    int quality = DEFAULT_QUALITY;
    switch (scaled_font->antialias_mode) {
    case CAIRO_ANTIALIAS_SUBPIXEL:
    case CAIRO_ANTIALIAS_BEST:
        quality = CLEARTYPE_QUALITY;
    }

    image = _compute_mask(&surface->base, quality);
    status = (cairo_int_status_t)image->status;
    if (status)
	goto FAIL;

    cairo_surface_set_device_offset (image, -x1, -y1);
    _cairo_scaled_glyph_set_surface (scaled_glyph,
				     &scaled_font->base,
				     (cairo_image_surface_t *) image);

  FAIL:
    cairo_surface_destroy (&surface->base);

    return status;
}

cairo_int_status_t
_cairo_dwrite_load_truetype_table(void                 *scaled_font,
				  unsigned long         tag,
				  long                  offset,
				  unsigned char        *buffer,
				  unsigned long        *length)
{
    cairo_dwrite_scaled_font_t *dwritesf = static_cast<cairo_dwrite_scaled_font_t*>(scaled_font);
    cairo_dwrite_font_face_t *face = reinterpret_cast<cairo_dwrite_font_face_t*>(dwritesf->base.font_face);

    const void *data;
    UINT32 size;
    void *tableContext;
    BOOL exists;
    face->dwriteface->TryGetFontTable(be32_to_cpu (tag),
				      &data,
				      &size,
				      &tableContext,
				      &exists);

    if (!exists) {
	return CAIRO_INT_STATUS_UNSUPPORTED;
    }

    if (buffer && *length && (UINT32)offset < size) {
        size = MIN(size - (UINT32)offset, *length);
        memcpy(buffer, (const char*)data + offset, size);
    }
    *length = size;

    if (tableContext) {
	face->dwriteface->ReleaseFontTable(tableContext);
    }
    return (cairo_int_status_t)CAIRO_STATUS_SUCCESS;
}

// WIN32 Helper Functions
cairo_font_face_t*
cairo_dwrite_font_face_create_for_dwrite_fontface (void* dwrite_font_face)
{
    IDWriteFontFace *dwriteface = static_cast<IDWriteFontFace*>(dwrite_font_face);
    cairo_dwrite_font_face_t *face = new cairo_dwrite_font_face_t;
    cairo_font_face_t *font_face;

    dwriteface->AddRef();
    
    face->dwriteface = dwriteface;
    face->font = NULL;

    font_face = (cairo_font_face_t*)face;
    
    _cairo_font_face_init (&((cairo_dwrite_font_face_t*)font_face)->base, &_cairo_dwrite_font_face_backend);

    return font_face;
}

cairo_int_status_t
_dwrite_draw_glyphs_to_gdi_surface_gdi(cairo_win32_surface_t *surface,
				       DWRITE_MATRIX *transform,
				       DWRITE_GLYPH_RUN *run,
				       COLORREF color,
				       cairo_dwrite_scaled_font_t *scaled_font,
				       const RECT &area)
{
    IDWriteGdiInterop *gdiInterop;
    DWriteFactory::Instance()->GetGdiInterop(&gdiInterop);
    IDWriteBitmapRenderTarget *rt;

    IDWriteRenderingParams *params =
        DWriteFactory::RenderingParams(scaled_font->rendering_mode);

    gdiInterop->CreateBitmapRenderTarget(surface->dc, 
					 area.right - area.left, 
					 area.bottom - area.top, 
					 &rt);

    /**
     * We set the number of pixels per DIP to 1.0. This is because we always want
     * to draw in device pixels, and not device independent pixels. On high DPI
     * systems this value will be higher than 1.0 and automatically upscale
     * fonts, we don't want this since we do our own upscaling for various reasons.
     */
    rt->SetPixelsPerDip(1.0);

    if (transform) {
	rt->SetCurrentTransform(transform);
    }
    BitBlt(rt->GetMemoryDC(),
	   0, 0,
	   area.right - area.left, area.bottom - area.top,
	   surface->dc,
	   area.left, area.top, 
	   SRCCOPY | NOMIRRORBITMAP);
    DWRITE_MEASURING_MODE measureMode; 
    switch (scaled_font->rendering_mode) {
    case cairo_dwrite_scaled_font_t::TEXT_RENDERING_GDI_CLASSIC:
    case cairo_dwrite_scaled_font_t::TEXT_RENDERING_NO_CLEARTYPE:
        measureMode = DWRITE_MEASURING_MODE_GDI_CLASSIC;
        break;
    default:
        measureMode = DWRITE_MEASURING_MODE_NATURAL;
        break;
    }
    HRESULT hr = rt->DrawGlyphRun(0, 0, measureMode, run, params, color);
    BitBlt(surface->dc,
	   area.left, area.top,
	   area.right - area.left, area.bottom - area.top,
	   rt->GetMemoryDC(),
	   0, 0, 
	   SRCCOPY | NOMIRRORBITMAP);
    params->Release();
    rt->Release();
    gdiInterop->Release();
    return CAIRO_INT_STATUS_SUCCESS;
}

#define ENHANCED_CONTRAST_REGISTRY_KEY \
    HKEY_CURRENT_USER, "Software\\Microsoft\\Avalon.Graphics\\DISPLAY1\\EnhancedContrastLevel"

void
DWriteFactory::CreateRenderingParams()
{
    if (!Instance()) {
	return;
    }

    Instance()->CreateRenderingParams(&mDefaultRenderingParams);

    // For EnhancedContrast, we override the default if the user has not set it
    // in the registry (by using the ClearType Tuner).
    FLOAT contrast;
    if (mEnhancedContrast >= 0.0 && mEnhancedContrast <= 10.0) {
	contrast = mEnhancedContrast;
    } else {
	HKEY hKey;
	if (RegOpenKeyExA(ENHANCED_CONTRAST_REGISTRY_KEY,
			  0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
	    contrast = mDefaultRenderingParams->GetEnhancedContrast();
	    RegCloseKey(hKey);
	} else {
	    contrast = 1.0;
	}
    }

    // For parameters that have not been explicitly set via the SetRenderingParams API,
    // we copy values from default params (or our overridden value for contrast)
    FLOAT gamma =
        mGamma >= 1.0 && mGamma <= 2.2 ?
            mGamma : mDefaultRenderingParams->GetGamma();
    FLOAT clearTypeLevel =
        mClearTypeLevel >= 0.0 && mClearTypeLevel <= 1.0 ?
            mClearTypeLevel : mDefaultRenderingParams->GetClearTypeLevel();
    DWRITE_PIXEL_GEOMETRY pixelGeometry =
        mPixelGeometry >= DWRITE_PIXEL_GEOMETRY_FLAT && mPixelGeometry <= DWRITE_PIXEL_GEOMETRY_BGR ?
            (DWRITE_PIXEL_GEOMETRY)mPixelGeometry : mDefaultRenderingParams->GetPixelGeometry();
    DWRITE_RENDERING_MODE renderingMode =
        mRenderingMode >= DWRITE_RENDERING_MODE_DEFAULT && mRenderingMode <= DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC ?
            (DWRITE_RENDERING_MODE)mRenderingMode : mDefaultRenderingParams->GetRenderingMode();
    Instance()->CreateCustomRenderingParams(gamma, contrast, clearTypeLevel,
	pixelGeometry, renderingMode,
	&mCustomClearTypeRenderingParams);
    Instance()->CreateCustomRenderingParams(gamma, contrast, clearTypeLevel,
        pixelGeometry, DWRITE_RENDERING_MODE_CLEARTYPE_GDI_CLASSIC,
        &mForceGDIClassicRenderingParams);
}

static cairo_bool_t
_cairo_dwrite_font_face_from_hdc (HDC hdc, /*IDWriteFontFace*/void **ppFontFace)
{
    RefPtr<IDWriteGdiInterop> gdiInterop;
    DWriteFactory::Instance()->GetGdiInterop(&gdiInterop);

    HRESULT hr = gdiInterop->CreateFontFaceFromHdc(hdc, reinterpret_cast<IDWriteFontFace**>(ppFontFace));
    return SUCCEEDED(hr);
}

static cairo_font_face_t *
_cairo_dwrite_font_face_create_for_hdc (HDC hdc)
{
    RefPtr<IDWriteFontFace> dwrite_font_face;

    cairo_bool_t result = _cairo_dwrite_font_face_from_hdc(hdc, reinterpret_cast<void**>(&dwrite_font_face));
    if (unlikely(!result)) {
        _cairo_error_throw(CAIRO_STATUS_NO_MEMORY);
        return (cairo_font_face_t *)&_cairo_font_face_nil;
    }
    return cairo_dwrite_font_face_create_for_dwrite_fontface (dwrite_font_face);
}

cairo_font_face_t *
cairo_dwrite_font_face_create_for_hfont (HFONT font)
{
    HDC hdc = GetDC(0);
    SelectObject(hdc, font);

    cairo_font_face_t* fontFace = _cairo_dwrite_font_face_create_for_hdc (hdc);

    ReleaseDC(0, hdc);
    return fontFace;
}

cairo_font_face_t *
cairo_dwrite_font_face_create_for_font_file (void *fontFile, int faceType)
{
    IDWriteFontFile *ff = static_cast<IDWriteFontFile*>(fontFile);

    RefPtr<IDWriteFontFace> fontFace;
    if (FAILED(DWriteFactory::Instance()->CreateFontFace(static_cast<DWRITE_FONT_FACE_TYPE>(faceType), 1, &ff, 0, DWRITE_FONT_SIMULATIONS_NONE, &fontFace))) {
        return nullptr;
    }
    return cairo_dwrite_font_face_create_for_dwrite_fontface (fontFace);
}

void *
cairo_dwrite_get_factory ()
{
    return DWriteFactory::Instance();
}
