#include "artdaq/ArtModules/BuildInfo_module.hh"

#include "artdaq/BuildInfo/GetPackageBuildInfo.hh"
#include "artdaq-core/BuildInfo/GetPackageBuildInfo.hh"
#include "artdaq-core-demo/BuildInfo/GetPackageBuildInfo.hh"
#include "artdaq-demo/BuildInfo/GetPackageBuildInfo.hh"

#include <string>

namespace demo
{
	static std::string instanceName = "ArtdaqDemoBuildInfo";
	typedef artdaq::BuildInfo<&instanceName, artdaqcore::GetPackageBuildInfo, artdaq::GetPackageBuildInfo, coredemo::GetPackageBuildInfo, demo::GetPackageBuildInfo> ArtdaqDemoBuildInfo;

	DEFINE_ART_MODULE(ArtdaqDemoBuildInfo)
}
