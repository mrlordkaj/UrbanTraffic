// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UrbanTraffic : ModuleRules
{
	public UrbanTraffic(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "PhysXVehicles"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        PublicDefinitions.Add("IA_MOVE_FORWARD=\"MoveForward\"");
        PublicDefinitions.Add("IA_MOVE_RIGHT=\"MoveRight\"");
        PublicDefinitions.Add("IA_MOVE_BRAKE=\"Brake\"");

        PublicDefinitions.Add("IA_INTERACTIVE=\"Interactive\"");
        PublicDefinitions.Add("IA_AUTONOMOUS=\"Autonomous\"");
        PublicDefinitions.Add("IA_FLASHLIGHT=\"Flashlight\"");

        PublicDefinitions.Add("IA_SIDELIGHT_L=\"SideLeft\"");
        PublicDefinitions.Add("IA_SIDELIGHT_R=\"SideRight\"");
        PublicDefinitions.Add("IA_SIDELIGHT_O=\"SideNone\"");

        PublicDefinitions.Add("IA_CAMERA_SWITCH=\"SwitchCamera\"");
        PublicDefinitions.Add("IA_CAMERA_ZOOM=\"ZoomCamera\"");
        PublicDefinitions.Add("IA_CAMERA_UP=\"LookUp\"");
        PublicDefinitions.Add("IA_CAMERA_RIGHT=\"LookRight\"");
    }
}
