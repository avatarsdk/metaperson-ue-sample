/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
* You may not use this file except in compliance with an authorized license
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
* See the License for the specific language governing permissions and limitations under the License.
* Written by Itseez3D, Inc. <support@avatarsdk.com>, December 2024
*/

#include "AvatarSDKMetaperson2Commands.h"

#define LOCTEXT_NAMESPACE "FAvatarSDKMetaperson2EditorModule"

void FAvatarSDKMetaperson2Commands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Avatar SDK Metaperson", "Import Avatar SDK Avatar", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
