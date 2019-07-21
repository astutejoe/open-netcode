using UnrealBuildTool;

public class FirstPersonServer : ModuleRules
{
	public FirstPersonServer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Sockets", "Networking", "NavigationSystem" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
