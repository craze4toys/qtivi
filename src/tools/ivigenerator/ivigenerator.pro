TEMPLATE = aux

!contains(CONFIG, no_internal_qface): include(qface_internal_build.pri)

# Make sure to only build this once in a debug_and_release config
# This needs to be the last step as it unsets other configs and may have side effects
CONFIG = release

templates_frontend.files +=  \
    templates_frontend/abstractmodule.cpp.tpl \
    templates_frontend/abstractmodule.h.tpl \
    templates_frontend/backendinterface.cpp.tpl \
    templates_frontend/backendinterface.h.tpl \
    templates_frontend/generated_comment.cpp.tpl \
    templates_frontend/global.h.tpl \
    templates_frontend/interface.cpp.tpl \
    templates_frontend/interface.h.tpl \
    templates_frontend/interface_p.h.tpl \
    templates_frontend/module.cpp.tpl \
    templates_frontend/module.h.tpl \
    templates_frontend/module.pri.tpl \
    templates_frontend/struct.cpp.tpl \
    templates_frontend/struct.h.tpl \
    templates_frontend/utils.tpl
templates_frontend.path = $$[QT_HOST_BINS]/ivigenerator/templates_frontend

templates_backend_simulator.files += \
    templates_backend_simulator/backend.cpp.tpl \
    templates_backend_simulator/backend.h.tpl \
    templates_backend_simulator/backend_range.cpp.tpl \
    templates_backend_simulator/generated_comment.cpp.tpl \
    templates_backend_simulator/plugin.cpp.tpl \
    templates_backend_simulator/plugin.h.tpl \
    templates_backend_simulator/plugin.json \
    templates_backend_simulator/plugin.pri.tpl \
    templates_backend_simulator/plugin.pro \
    templates_backend_simulator/utils.tpl
templates_backend_simulator.path = $$[QT_HOST_BINS]/ivigenerator/templates_backend_simulator

generator.files += \
    generate.py \
    templates_frontend.yaml \
    templates_backend_simulator.yaml
generator.path = $$[QT_HOST_BINS]/ivigenerator

INSTALLS += templates_frontend templates_backend_simulator generator

# Ensure files are installed to qtbase for non-prefixed builds
!force_independent:if(!debug_and_release|!build_all|CONFIG(release, debug|release)) {
    for (install_target, INSTALLS) {
        path = $$eval($${install_target}.path)
        $${install_target}_copy.input = $${install_target}.files
        $${install_target}_copy.output = $$path/${QMAKE_FILE_IN_BASE}${QMAKE_FILE_EXT}
        contains($${install_target}.CONFIG, directory): $${install_target}_copy.commands = $$QMAKE_COPY_DIR ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
        else: $${install_target}_copy.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
        $${install_target}_copy.name = COPY ${QMAKE_FILE_IN}
        $${install_target}_copy.CONFIG = no_link target_predeps no_clean
        QMAKE_EXTRA_COMPILERS += $${install_target}_copy
    }
}
