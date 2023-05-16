/* Copyright (C) Itseez3D, Inc. - All Rights Reserved
 * You may not use this file except in compliance with an authorized license
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * UNLESS REQUIRED BY APPLICABLE LAW OR AGREED BY ITSEEZ3D, INC. IN WRITING, SOFTWARE DISTRIBUTED UNDER THE LICENSE IS DISTRIBUTED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED
 * See the License for the specific language governing permissions and limitations under the License.
 * Written by Itseez3D, Inc. <support@itseez3D.com>, February 2023
 */

#include "ZipHelper.h"
#include "Misc/Paths.h"
#include <string>
#include <vector>
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"

bool ZipHelper::VerifyOrCreateDirectory(const FString& TestDir)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    // Directory Exists?
    if (!PlatformFile.DirectoryExists(*TestDir))
    {
        PlatformFile.CreateDirectory(*TestDir);

        if (!PlatformFile.DirectoryExists(*TestDir))
        {
            return false;
        }
    }
    return true;
}

void ZipHelper::Unzip(const FString& uinFile, const FString& uoutDir, bool toSubfolder, bool removeArchive)
{
    std::string zipFile;
    std::string outDir;
    zipFile = std::string(TCHAR_TO_UTF8(*uinFile));
    if (toSubfolder)
    {
        FString subdirName = FPaths::GetBaseFilename(uinFile);
        FString subdir = FPaths::Combine(uoutDir, subdirName);
        VerifyOrCreateDirectory(subdir);
        outDir = std::string(TCHAR_TO_UTF8(*subdir));
    }
    else 
    {
        outDir = std::string(TCHAR_TO_UTF8(*uoutDir));
    }

    std::vector<std::string> files = {};
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    auto status = mz_zip_reader_init_file(&zip_archive, zipFile.c_str(), 0);
    if (!status) return;// files;
    int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);
    if (fileCount == 0)
    {
        mz_zip_reader_end(&zip_archive);
        return;// files;
    }
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&zip_archive, 0, &file_stat))
    {
        mz_zip_reader_end(&zip_archive);
        return;// files;
    }

    // Get and print information about each file in the archive.
    for (int i = 0; i < fileCount; i++)
    {
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) continue;
        if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) 
        { // skip directories for now
            std::string archiveDir = outDir + '/' + file_stat.m_filename;
            VerifyOrCreateDirectory(FString(archiveDir.c_str()));
        }
        std::string destFile = outDir + '/' + file_stat.m_filename;
        if (mz_zip_reader_extract_to_file(&zip_archive, i, destFile.c_str(), 0))
        {
            files.emplace_back(destFile);
        }
    }

    // Close the archive, freeing any resources it was using
    mz_zip_reader_end(&zip_archive);
    //return files;
    if (removeArchive) {
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        PlatformFile.DeleteFile(*uinFile);
    }
}