cmake_minimum_required(VERSION 2.8.12)

set(NAME "icuin")
project(${NAME})

set(I18N_SOURCES
    alphaindex.cpp
    anytrans.cpp
    astro.cpp
    basictz.cpp
    bocsu.cpp
    brktrans.cpp
    buddhcal.cpp
    calendar.cpp
    casetrn.cpp
    cecal.cpp
    chnsecal.cpp
    choicfmt.cpp
    coleitr.cpp
    coll.cpp
    compactdecimalformat.cpp
    coptccal.cpp
    cpdtrans.cpp
    csdetect.cpp
    csmatch.cpp
    csr2022.cpp
    csrecog.cpp
    csrmbcs.cpp
    csrsbcs.cpp
    csrucode.cpp
    csrutf8.cpp
    curramt.cpp
    currfmt.cpp
    currpinf.cpp
    currunit.cpp
    dangical.cpp
    datefmt.cpp
    dcfmtsym.cpp
    decContext.c
    decfmtst.cpp
    decimfmt.cpp
    decNumber.c
    digitlst.cpp
    dtfmtsym.cpp
    dtitvfmt.cpp
    dtitvinf.cpp
    dtptngen.cpp
    dtrule.cpp
    esctrn.cpp
    ethpccal.cpp
    fmtable.cpp
    fmtable_cnv.cpp
    format.cpp
    fphdlimp.cpp
    fpositer.cpp
    funcrepl.cpp
    gender.cpp
    gregocal.cpp
    gregoimp.cpp
    hebrwcal.cpp
    identifier_info.cpp
    indiancal.cpp
    inputext.cpp
    islamcal.cpp
    japancal.cpp
    locdspnm.cpp
    measfmt.cpp
    measure.cpp
    msgfmt.cpp
    name2uni.cpp
    nfrs.cpp
    nfrule.cpp
    nfsubs.cpp
    nortrans.cpp
    nultrans.cpp
    numfmt.cpp
    numsys.cpp
    olsontz.cpp
    persncal.cpp
    plurfmt.cpp
    plurrule.cpp
    quant.cpp
    rbnf.cpp
    rbt.cpp
    rbt_data.cpp
    rbt_pars.cpp
    rbt_rule.cpp
    rbt_set.cpp
    rbtz.cpp
    regexcmp.cpp
    regeximp.cpp
    regexst.cpp
    regextxt.cpp
    region.cpp
    reldtfmt.cpp
    rematch.cpp
    remtrans.cpp
    repattrn.cpp
    scriptset.cpp
    search.cpp
    selfmt.cpp
    simpletz.cpp
    smpdtfmt.cpp
    smpdtfst.cpp
    sortkey.cpp
    strmatch.cpp
    strrepl.cpp
    stsearch.cpp
    taiwncal.cpp
    tblcoll.cpp
    timezone.cpp
    titletrn.cpp
    tmunit.cpp
    tmutamt.cpp
    tmutfmt.cpp
    tolowtrn.cpp
    toupptrn.cpp
    translit.cpp
    transreg.cpp
    tridpars.cpp
    tzfmt.cpp
    tzgnames.cpp
    tznames.cpp
    tznames_impl.cpp
    tzrule.cpp
    tztrans.cpp
    ucal.cpp
    ucln_in.c
    ucol.cpp
    ucol_bld.cpp
    ucol_cnt.cpp
    ucol_elm.cpp
    ucol_res.cpp
    ucol_sit.cpp
    ucol_tok.cpp
    ucol_wgt.cpp
    ucoleitr.cpp
    ucsdet.cpp
    ucurr.cpp
    udat.cpp
    udateintervalformat.cpp
    udatpg.cpp
    ulocdata.c
    umsg.cpp
    unesctrn.cpp
    uni2name.cpp
    unum.cpp
    unumsys.cpp
    upluralrules.cpp
    uregex.cpp
    uregexc.cpp
    uregion.cpp
    usearch.cpp
    uspoof.cpp
    uspoof_build.cpp
    uspoof_conf.cpp
    uspoof_impl.cpp
    uspoof_wsconf.cpp
    utmscale.c
    utrans.cpp
    vtzone.cpp
    vzone.cpp
    windtfmt.cpp
    winnmfmt.cpp
    wintzimpl.cpp
    zonemeta.cpp
    zrule.cpp
    ztrans.cpp
)

include_directories(
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
)

link_directories(
    ${OUTPUT_DIR}
)

add_definitions(-DU_DISABLE_RENAMING=1 -DU_I18N_IMPLEMENTATION -DICU_BUILD=1 -DU_ICU_ENTRY_POINT_RENAME=1 -DU_USING_ICU_NAMESPACE=0 -DHAVE_DLOPEN=0 -DUCONFIG_NO_NON_HTML5_CONVERSION=1)

if (WIN32)
    add_library(${NAME} SHARED ${I18N_SOURCES})

    set(LIBRARY_DEPS
        icuuc
        ${WIN32_SYSTEM_LIBRARIES}
    )

    target_link_libraries(${NAME} ${LIBRARY_DEPS})
elseif (ANDROID)
    add_library(${NAME} STATIC ${I18N_SOURCES})
endif ()

add_post_build_command(icuin)

copy_library_headers_directory(icuin unicode include/unicode)
