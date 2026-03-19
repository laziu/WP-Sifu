#pragma once

#include "CoreMinimal.h"


namespace Ext
{
	// --- ConstructorHelpers::FObjectFinder helper functions ---

	template <typename T>
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


	// --- ConstructorHelpers::FClassFinder helper functions ---

	template <typename T>
	bool SetClass(TSubclassOf<T>& Target, const TCHAR* ClassToFind)
	{
		if (ConstructorHelpers::FClassFinder<T> Class(ClassToFind); Class.Succeeded())
		{
			Target = Class.Class;
			return true;
		}
		return false;
	}

	template <class T>
	TSubclassOf<T> OpenClass(const TCHAR* ClassToFind, const TSubclassOf<T>& FailureValue = nullptr)
	{
		if (ConstructorHelpers::FClassFinder<T> Class(ClassToFind); Class.Succeeded())
		{
			return Class.Class;
		}
		return FailureValue;
	}

	template <class T>
	TSubclassOf<T> OpenClassChecked(const TCHAR* ClassToFind)
	{
		ConstructorHelpers::FClassFinder<T> Class(ClassToFind);
		check(Class.Succeeded());
		return Class.Class;
	}


	// Helper class for UObject
	class Bind final
	{
		TObjectPtr<UObject> Owner;

	public:
		Bind(const TObjectPtr<UObject>& Owner) : Owner(Owner)
		{
		}

		// CreateDefaultSubobject helper function
		template <class T>
		void CreateSubobject(TObjectPtr<T>& Target, FName ObjectName) const
		{
			Target = Owner->CreateDefaultSubobject<T>(ObjectName);
		}
	};
}

#define EXT_CREATE_SUBOBJECT(Target) \
{ \
	Ext::Bind(this).CreateSubobject(Target, TEXT(#Target)); \
}
