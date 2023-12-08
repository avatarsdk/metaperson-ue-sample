/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#include "AvatarSDKMetaperson2.h"
#include "AvatarSDKRuntimeSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"


#define LOCTEXT_NAMESPACE "FAvatarSDKMetaperson2Module"

void FAvatarSDKMetaperson2Module::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Avatar SDK Metaperson 2",
			LOCTEXT("AvatarSDKSettingsName", "Avatar SDK Metaperson 2"),
			LOCTEXT("AvatarSDKSettingsDescription", "Configure the Avatar SDK plugin."),
			GetMutableDefault<UAvatarSDKRuntimeSettings>()
		);

	}
}

void FAvatarSDKMetaperson2Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAvatarSDKMetaperson2Module, AvatarSDKMetaperson2)