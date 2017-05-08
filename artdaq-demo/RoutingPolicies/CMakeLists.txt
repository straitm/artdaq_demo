simple_plugin("MisbehaviorTest" "policy"
    artdaq_Application_Routing
    )

simple_plugin(NthEvent "policy"
artdaq_Application_Routing
)
install_headers()
install_fhicl(SUBDIRS fcl)
install_source()
