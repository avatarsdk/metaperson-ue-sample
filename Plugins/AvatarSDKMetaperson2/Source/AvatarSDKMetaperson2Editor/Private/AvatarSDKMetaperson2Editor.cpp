/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, May 2023
 */

#include "AvatarSDKMetaperson2Editor.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "AvatarSDKMetaperson2Import.h"
#include "AvatarSDK2MetapersonStyle.h"
#include "AvatarSDKMetaperson2Commands.h"

static const FName TabName("AvatarSDKMetaperson2");

#define LOCTEXT_NAMESPACE "FAvatarSDKMetaperson2EditorModule"

void FAvatarSDKMetaperson2EditorModule::StartupModule()
{
	FAvatarSDK2MetapersonStyle::Initialize();
	FAvatarSDK2MetapersonStyle::ReloadTextures();

	FAvatarSDKMetaperson2Commands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FAvatarSDKMetaperson2Commands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FAvatarSDKMetaperson2EditorModule::PluginButtonClicked),
		FCanExecuteAction());


	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAvatarSDKMetaperson2EditorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, FOnSpawnTab::CreateRaw(this, &FAvatarSDKMetaperson2EditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FAvatarSdkRaTabTitle", "Avatar SDK Realistic Animation"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
 }

void FAvatarSDKMetaperson2EditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAvatarSDK2MetapersonStyle::Shutdown();

	FAvatarSDKMetaperson2Commands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
}

void FAvatarSDKMetaperson2EditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(TabName);

}

void FAvatarSDKMetaperson2EditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FAvatarSDKMetaperson2Commands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAvatarSDKMetaperson2Commands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

TSharedRef<class SDockTab> FAvatarSDKMetaperson2EditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)[
		SNew(SAvatarSDKMetaperson2Import)
	];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAvatarSDKMetaperson2EditorModule, AvatarSDKMetaperson2Editor)