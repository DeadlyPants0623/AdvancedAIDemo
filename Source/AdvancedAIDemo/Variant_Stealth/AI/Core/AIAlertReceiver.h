#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AIAlertTypes.h"
#include "AIAlertReceiver.generated.h"

UINTERFACE(BlueprintType)
class ADVANCEDAIDEMO_API UAIAlertReceiver : public UInterface
{
	GENERATED_BODY()
};

class ADVANCEDAIDEMO_API IAIAlertReceiver
{
	GENERATED_BODY()

public:
	// Must match your controller's override signature exactly
	virtual bool ReceiveAIAlert(const FAIAlertData& AlertData) = 0;
};