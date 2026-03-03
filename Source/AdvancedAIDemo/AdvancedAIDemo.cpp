// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdvancedAIDemo.h"
#include "Modules/ModuleManager.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GameplayDebuggerCategory_Suspicion.h"
#endif

class FAdvancedAIDemoModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

#if WITH_GAMEPLAY_DEBUGGER
		// Ensure the module is loaded before using IGameplayDebugger::Get()
		FModuleManager::LoadModuleChecked<FDefaultModuleImpl>("GameplayDebugger");

		if (IGameplayDebugger::IsAvailable())
		{
			UE_LOG(LogTemp, Warning, TEXT("Registering SuspicionAI Gameplay Debugger category"));

			IGameplayDebugger& Debugger = IGameplayDebugger::Get();
			Debugger.RegisterCategory(
				TEXT("SuspicionAI"),
				IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Suspicion::MakeInstance),
				EGameplayDebuggerCategoryState::EnabledInGameAndSimulate,
				5
			);

			Debugger.NotifyCategoriesChanged();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Gameplay Debugger ENABLED but IGameplayDebugger not available"));
		}
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FAdvancedAIDemoModule, AdvancedAIDemo, "AdvancedAIDemo");

DEFINE_LOG_CATEGORY(LogAdvancedAIDemo)