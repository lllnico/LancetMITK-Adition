project(org_mitk_lancet_femurprecisionregistrations)

mitk_create_plugin(
  EXPORT_DIRECTIVE FEMURPRECISIONREGISTRATIONS_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
