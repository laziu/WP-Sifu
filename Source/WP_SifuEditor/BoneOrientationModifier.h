// BoneOrientationModifier.h
#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "BoneOrientationModifier.generated.h"

class UAnimSequence;

UCLASS(meta = (DisplayName = "Bone Orientation Modifier"))
class WP_SIFUEDITOR_API UBoneOrientationModifier : public UAnimationModifier
{
	GENERATED_BODY()

public:
	// 회전시킬 본 이름. UE 기본 스켈레톤은 "root"
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName BoneName = FName("root");

	// Z축(Yaw) 회전량. 양수 = CCW
	UPROPERTY(EditAnywhere, Category = "Settings")
	float YawDegrees = 90.f;

	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

private:
	void RotateBoneTrack(UAnimSequence* AnimationSequence, const FQuat& Delta);
};
