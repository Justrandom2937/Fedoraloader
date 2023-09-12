#include "Loader.h"
#include "Web/Web.h"
#include "Zip/Zip.h"
#include "LoadLibrary/LoadLibrary.h"
#include "ManualMap/ManualMap.h"
#include "../Utils/Utils.h"

#include <fstream>
#include <stdexcept>

LPCWSTR ACTION_URL = L"https://nightly.link/Fedoraware/Fedoraware/workflows/msbuild/main/Fedoraware.zip";
LPCSTR DLL_FILE_NAME = "Fware-Release.dll";

// Retrieves the Fware binary from web/disk
BinData GetBinary(const LaunchInfo& launchInfo)
{
	BinData binary;

	// Read the file from web/disk
	if (launchInfo.File)
	{
		binary = Utils::ReadBinaryFile(launchInfo.File);
	}
	else
	{
		const LPCWSTR url = launchInfo.URL ? launchInfo.URL : ACTION_URL;
		binary = Web::DownloadFile(url);
	}

	// Check if the file is packed
	const auto* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(binary.Data);
	if (dosHeader->e_magic == 0x4B50)
	{
		Zip::UnpackFile(binary, DLL_FILE_NAME);
	}

	return binary;
}

// Loads and injects Fware
bool Loader::Load(const LaunchInfo& launchInfo)
{
	// Retrieve the binary
	const BinData binary = GetBinary(launchInfo);
	if (!binary.Data || binary.Size < 0x1000)
	{
		throw std::runtime_error("Invalid binary");
	}

	// Find the game
	const HANDLE hGame = Utils::GetProcessHandle("hl2.exe");
	if (hGame == INVALID_HANDLE_VALUE || hGame == nullptr)
	{
		throw std::runtime_error("Failed to get game handle");
	}

	// Inject the binary
	const bool result = MM::Inject(hGame, binary);

	// Cleanup
	delete[] binary.Data;
	return result;
}

bool Loader::Debug(const LaunchInfo& launchInfo)
{
	// Find the game
	const HANDLE hGame = Utils::GetProcessHandle("hl2.exe");
	if (hGame == INVALID_HANDLE_VALUE || hGame == nullptr)
	{
		throw std::runtime_error("Failed to get game handle");
	}

	// Inject the binary
	const bool result = LL::Inject(hGame, launchInfo.File);
	return result;
}
