using UnrealBuildTool;

public class FirstPersonClient : ModuleRules
{
	public FirstPersonClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Sockets", "Networking", "NavigationSystem" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
