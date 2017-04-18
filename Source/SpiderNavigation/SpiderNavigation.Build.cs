using UnrealBuildTool;

public class SpiderNavigation : ModuleRules
{
    public SpiderNavigation(TargetInfo Target)
	{
        PrivateIncludePaths.Add("SpiderNavigation/Private");

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "UnrealEd"
            });


        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
                "InputCore",
                "UnrealEd"
            }
			);
	}
}
