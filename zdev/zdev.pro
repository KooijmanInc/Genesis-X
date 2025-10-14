TEMPLATE = aux
DISTFILES += \
    $$files(../docs/*, true) \
    $$files($$PWD/../tools/*, true) \
    $$files($$PWD/../mkspecs/*, true) \
    $$files($$PWD/../config/*, true) \
    $$files($$PWD/../scripts/*, true) \
    ../.github/FUNDING.yml \
    ../.github/PULL_REQUEST_TEMPLATE.md \
    ../.github/ISSUE_TEMPLATE/bug_report.md \
    ../.github/ISSUE_TEMPLATE/feature_request.md \
    ../.github/ISSUE_TEMPLATE/config.yml \
    ../.github/workflows/quality.yml \
    ../.github/workflows/collect-traffic.yml \
    ../.gitlab-ci.yml \
    ../.gitignore \
    ../.gitattributes
