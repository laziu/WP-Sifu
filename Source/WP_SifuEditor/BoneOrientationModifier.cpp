// BoneOrientationModifier.cpp
#include "BoneOrientationModifier.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimSequence.h"

void UBoneOrientationModifier::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnApply_Implementation(AnimationSequence);

	const FQuat Delta(FVector::UpVector, FMath::DegreesToRadians(YawDegrees));
	RotateBoneTrack(AnimationSequence, Delta);
}

void UBoneOrientationModifier::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	Super::OnRevert_Implementation(AnimationSequence);

	// Apply의 역방향으로 복구
	const FQuat InverseDelta(FVector::UpVector, FMath::DegreesToRadians(-YawDegrees));
	RotateBoneTrack(AnimationSequence, InverseDelta);
}

void UBoneOrientationModifier::RotateBoneTrack(UAnimSequence* AnimationSequence, const FQuat& Delta)
{
	if (!AnimationSequence)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BoneOrientationModifier: AnimationSequence is null."));
		return;
	}

	IAnimationDataModel* DataModel = AnimationSequence->GetDataModel();
	if (!DataModel || !DataModel->IsValidBoneTrackName(BoneName))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BoneOrientationModifier: '%s' 본 트랙을 찾을 수 없습니다."),
			*BoneName.ToString());
		return;
	}

	TArray<FTransform> BoneTransforms;
	DataModel->GetBoneTrackTransforms(BoneName, BoneTransforms);

	if (BoneTransforms.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BoneOrientationModifier: '%s' 본에 키가 없습니다."),
			*BoneName.ToString());
		return;
	}

	TArray<FVector> PosKeys;
	TArray<FQuat> RotKeys;
	TArray<FVector> ScaleKeys;
	PosKeys.Reserve(BoneTransforms.Num());
	RotKeys.Reserve(BoneTransforms.Num());
	ScaleKeys.Reserve(BoneTransforms.Num());

	for (const FTransform& BoneTransform : BoneTransforms)
	{
		RotKeys.Add((Delta * BoneTransform.GetRotation()).GetNormalized());
		PosKeys.Add(Delta.RotateVector(BoneTransform.GetTranslation()));
		ScaleKeys.Add(BoneTransform.GetScale3D());
	}

	IAnimationDataController& Controller = AnimationSequence->GetController();
	Controller.SetBoneTrackKeys(BoneName, PosKeys, RotKeys, ScaleKeys);
}
