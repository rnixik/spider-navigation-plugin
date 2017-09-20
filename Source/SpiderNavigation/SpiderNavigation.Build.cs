using UnrealBuildTool;

public class SpiderNavigation : ModuleRules
{
    public SpiderNavigation(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateIncludePaths.Add("SpiderNavigation/Private");

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine"
            });

        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine"
            });
	}
}
