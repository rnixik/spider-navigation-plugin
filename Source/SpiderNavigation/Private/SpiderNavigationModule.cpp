#include "SpiderNavigationModule.h"
#include "Modules/ModuleManager.h"
#include "SpiderNavigationPrivate.h"

//////////////////////////////////////////////////////////////////////////
// FSpiderNavigationModule

class FSpiderNavigationModule : public ISpiderNavigationModuleInterface
{
public:
	virtual void StartupModule() override
	{
		check(GConfig);		
		UE_LOG(LogSpiderNavigation, Log, TEXT("Hi from Spider Navigation"))
	}

	virtual void ShutdownModule() override
	{
	}
};

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_MODULE(FSpiderNavigationModule, SpiderNavigation);
DEFINE_LOG_CATEGORY(LogSpiderNavigation);
