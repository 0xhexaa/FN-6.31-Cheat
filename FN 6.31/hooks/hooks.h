#pragma once

namespace Hooks {

	bool Init();
	void Destroy();

	bool CreateHook(void* Target, void* Detour, void** Original);
	bool EnableHook(void* Target);
	bool RemoveHook(void* Target);
	bool DisableHook(void* Target);

}

