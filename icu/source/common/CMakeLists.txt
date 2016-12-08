cmake_minimum_required(VERSION 2.8.12)

set(NAME "icuuc")
project(${NAME})

set(COMMON_SOURCES
    appendable.cpp
    bmpset.cpp
    brkeng.cpp
    brkiter.cpp
    bytestream.cpp
    bytestrie.cpp
    bytestriebuilder.cpp
    bytestrieiterator.cpp
    caniter.cpp
    chariter.cpp
    charstr.cpp
    cmemory.c
    cstring.c
    cwchar.c
    dictbe.cpp
    dictionarydata.cpp
    dtintrv.cpp
    errorcode.cpp
    filterednormalizer2.cpp
    icudataver.c
    icuplug.c
    listformatter.cpp
    locavailable.cpp
    locbased.cpp
    locdispnames.cpp
    locid.cpp
    loclikely.cpp
    locmap.c
    locresdata.cpp
    locutil.cpp
    messagepattern.cpp
    normalizer2.cpp
    normalizer2impl.cpp
    normlzr.cpp
    parsepos.cpp
    patternprops.cpp
    propname.cpp
    propsvec.c
    punycode.cpp
    putil.cpp
    rbbi.cpp
    rbbidata.cpp
    rbbinode.cpp
    rbbirb.cpp
    rbbiscan.cpp
    rbbisetb.cpp
    rbbistbl.cpp
    rbbitblb.cpp
    resbund.cpp
    resbund_cnv.cpp
    ruleiter.cpp
    schriter.cpp
    serv.cpp
    servlk.cpp
    servlkf.cpp
    servls.cpp
    servnotf.cpp
    servrbf.cpp
    servslkf.cpp
    stringpiece.cpp
    stringtriebuilder.cpp
    uarrsort.c
    ubidi.c
    ubidi_props.c
    ubidiln.c
    ubidiwrt.c
    ubrk.cpp
    ucase.cpp
    ucasemap.cpp
    ucasemap_titlecase_brkiter.cpp
    ucat.c
    uchar.c
    ucharstrie.cpp
    ucharstriebuilder.cpp
    ucharstrieiterator.cpp
    uchriter.cpp
    ucln_cmn.c
    ucmndata.c
    ucnv.c
    ucnv_bld.cpp
    ucnv_cb.c
    ucnv_cnv.c
    ucnv_ct.c
    ucnv_err.c
    ucnv_ext.cpp
    ucnv_io.cpp
    ucnv_lmb.c
    ucnv_set.c
    ucnv_u16.c
    ucnv_u32.c
    ucnv_u7.c
    ucnv_u8.c
    ucnv2022.cpp
    ucnvbocu.cpp
    ucnvdisp.c
    ucnvhz.c
    ucnvisci.c
    ucnvlat1.c
    ucnvmbcs.c
    ucnvscsu.c
    ucnvsel.cpp
    ucol_swp.cpp
    udata.cpp
    udatamem.c
    udataswp.c
    uenum.c
    uhash.c
    uhash_us.cpp
    uidna.cpp
    uinit.cpp
    uinvchar.c
    uiter.cpp
    ulist.c
    uloc.cpp
    uloc_tag.c
    umapfile.c
    umath.c
    umutex.cpp
    unames.cpp
    unifilt.cpp
    unifunct.cpp
    uniset.cpp
    uniset_closure.cpp
    uniset_props.cpp
    unisetspan.cpp
    unistr.cpp
    unistr_case.cpp
    unistr_case_locale.cpp
    unistr_cnv.cpp
    unistr_props.cpp
    unistr_titlecase_brkiter.cpp
    unorm.cpp
    unorm_it.c
    unormcmp.cpp
    uobject.cpp
    uprops.cpp
    ures_cnv.c
    uresbund.cpp
    uresdata.c
    usc_impl.c
    uscript.c
    uscript_props.cpp
    uset.cpp
    uset_props.cpp
    usetiter.cpp
    ushape.cpp
    usprep.cpp
    ustack.cpp
    ustr_cnv.c
    ustr_titlecase_brkiter.cpp
    ustr_wcs.cpp
    ustrcase.cpp
    ustrcase_locale.cpp
    ustrenum.cpp
    ustrfmt.c
    ustring.cpp
    ustrtrns.cpp
    utext.cpp
    utf_impl.c
    util.cpp
    util_props.cpp
    utrace.c
    utrie.cpp
    utrie2.cpp
    utrie2_builder.cpp
    uts46.cpp
    utypes.c
    uvector.cpp
    uvectr32.cpp
    uvectr64.cpp
    wintz.c
)

include_directories(
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
)

link_directories(
    ${OUTPUT_DIR}
)

add_definitions(-DU_DISABLE_RENAMING=1 -DU_COMMON_IMPLEMENTATION -DICU_BUILD=1 -DU_ICU_ENTRY_POINT_RENAME=1 -DU_USING_ICU_NAMESPACE=0 -DHAVE_DLOPEN=0 -DUCONFIG_NO_NON_HTML5_CONVERSION=1)

if (WIN32)
    add_library(${NAME} SHARED ${COMMON_SOURCES})

    set(LIBRARY_DEPS
        icudt
        ${WIN32_SYSTEM_LIBRARIES}
    )

    target_link_libraries(${NAME} ${LIBRARY_DEPS})
elseif (ANDROID)
    add_library(${NAME} STATIC ${COMMON_SOURCES})
endif ()

add_post_build_command(icuuc)

copy_library_headers_directory(icuuc unicode include/unicode)
