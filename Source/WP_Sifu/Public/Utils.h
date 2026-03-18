#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "Utils.generated.h"


// helper codes
namespace Utils
{
	template <class T>
	bool SetObject(TObjectPtr<T>& Target, const TCHAR* ObjectToFind)
	{
		if (ConstructorHelpers::FObjectFinder<T> Object(ObjectToFind); Object.Succeeded())
		{
			Target = Object.Object;
			return true;
		}
		return false;
	}

	template <class T>
	TObjectPtr<T> OpenObject(const TCHAR* ObjectToFind, const TObjectPtr<T>& FailureValue = nullptr)
	{
		if (ConstructorHelpers::FObjectFinder<T> Object(ObjectToFind); Object.Succeeded())
		{
			return Object.Object;
		}
		return FailureValue;
	}

	template <class T>
	TObjectPtr<T> OpenObjectChecked(const TCHAR* ObjectToFind)
	{
		ConstructorHelpers::FObjectFinder<T> Object(ObjectToFind);
		check(Object.Succeeded());
		return Object.Object;
	}

	template <class T>
	bool WithObject(const TCHAR* ObjectToFind, std::function<void(TObjectPtr<T>&)> SuccessFunction)
	{
		if (ConstructorHelpers::FObjectFinder<T> Object(ObjectToFind); Object.Succeeded())
		{
			SuccessFunction(Object.Object);
			return true;
		}
		return false;
	}
}
