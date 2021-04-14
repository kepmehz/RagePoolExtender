#include "pch.h"

using GetSizeOfPool = int64_t(*)(int64_t config, uint32_t poolhash, uint32_t defaultSize);
void* g_GetSizeOfPool{};
GetSizeOfPool ogGetSizeOfPool{};
HMODULE g_module;
bool extended = false;
int64_t fwScriptGuidValue;
int64_t atDScriptObjectNodeValue;
int64_t CGameScriptHandlerValue;
int64_t CScriptEntityExtensionValue;

int64_t GetSizeOfPoolHook(int64_t config, uint32_t poolHash, uint32_t defaultSize)
{
	int64_t result{};

	switch (poolHash)
	{
	case 0x7311A8D7: // joaat("fwScriptGuid")
		{
		result = fwScriptGuidValue;
		}
		break;
	case 0x3EEA2DA9: // joaat("atDScriptObjectNode")
		{
		result = atDScriptObjectNodeValue;
		extended = true; // last in queue
		}
		break;
	case 0x1182232C: // joaat("CGameScriptHandler")
		{
		result = CGameScriptHandlerValue;
		}
		break;
	case 0xEF7129CB: // joaat("CScriptEntityExtension")
		{
		result = CScriptEntityExtensionValue;
		}
		break;
	}

	if (result > defaultSize)
		return result;

	return ogGetSizeOfPool(config, poolHash, defaultSize);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		g_module = hModule;
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, [](PVOID) -> DWORD
			{
				if (!std::filesystem::exists("./config.ini"))
				{
					CSimpleIniA ini;
					ini.SetLongValue("Pools", "fwScriptGuid", 3072);
					ini.SetLongValue("Pools", "atDScriptObjectNode", 3072);
					ini.SetLongValue("Pools", "CGameScriptHandler", 256);
					ini.SetLongValue("Pools", "CScriptEntityExtension", 3072);
					ini.SaveFile("./config.ini");
				}

				CSimpleIniA ini;
				if (ini.LoadFile("./config.ini") == SI_OK)
				{
					fwScriptGuidValue = ini.GetLongValue("Pools", "fwScriptGuid");
					atDScriptObjectNodeValue = ini.GetLongValue("Pools", "atDScriptObjectNode");
					CGameScriptHandlerValue = ini.GetLongValue("Pools", "CGameScriptHandler");
					CScriptEntityExtensionValue = ini.GetLongValue("Pools", "CScriptEntityExtension");
				}
			
				auto sc = new scanner(nullptr);
				auto func = sc->scan("83 79 10 00 44 8B D2");
				delete sc;

				if (func)
				{
					g_GetSizeOfPool = func;
					if (MH_Initialize() == MH_OK)
						if (MH_CreateHook(g_GetSizeOfPool, GetSizeOfPoolHook, reinterpret_cast<void**>(&ogGetSizeOfPool)) == MH_OK)
							MH_EnableHook(g_GetSizeOfPool);
				}

				while (!extended)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
				}

				MH_Uninitialize();
				MH_DisableHook(g_GetSizeOfPool);
				MH_RemoveHook(g_GetSizeOfPool);

				FreeLibraryAndExitThread(g_module, 0);

				return 0;
			}, nullptr, 0, nullptr);
	}
	return TRUE;
}

