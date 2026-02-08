#pragma once
#pragma warning (disable : 4275)
#include <exception>
#include <stdexcept>
#include "Smasher_export.h"
#define SMASHER_API SMASHERENGINE_EXPORT

#define SMASHER_EXCEPTION(name) class name : public std::exception { \
public: \
explicit name() : m_Message("") {} \
explicit name(const std::string &msg) : m_Message(msg) {} \
const char *what() const noexcept {return m_Message.c_str();} \
private: \
std::string m_Message; \
}; \

namespace Smasher {

	using ErrorCode = int;

	/*
	* 
	* Expected<T> is used in leau of exceptions and acts
	* very similar to error op-codes. Expected<T> stores
	* either the returned value of a function OR the
	* error code from a function
	* 
	* 
	* For example, let's say we have a malloc function
	* 
	* void *malloc(size_t bytes)
	* 
	* Since this has the potential for runtime errors we 
	* can rewrite it as 
	* 
	* Expected<void *> malloc(size_t bytes)
	* 
	* Now if there is a runtime error within the function
	* we return
	* 
	* return Expected<void *>::ErrorCode<YOUR_ERROR_CODE>
	* 
	* If the function was successful we can return the void*
	* like normal.
	* 
	* return ptr;
	* 
	* Now when we examine the return we can check if there was
	* an error like so
	* 
	* Expected<void *>ret = malloc(-1);
	* if (ret.HasError()) {
	*     std::cout << ret.GetError();
	* }
	* else {
	*    void* ptr = ret; // ret is implicity cast to void *
	* }
	*/
	template<class T>
	class Expected {
	private:
		// Used to indicate that we the "Expected" object
		// will be housing an error not a value
		class ErrTAG {};

	public:
		Expected(const Expected<T>& other) = delete;
		Expected(Expected<T>&& other) = delete;
		Expected<T>& operator= (const Expected<T>& other) = delete;
		Expected<T>& operator= (Expected<T>&& other) = delete;

		Expected(const T& other);
		Expected(T&& other);
		Expected<T>& operator= (const T& other);
		Expected<T>& operator= (T&& other);
		~Expected();

		// Is there a value stored in Exepected?
		operator bool() const;

		// Factory Method for throwing error
		static Expected<T> Error(ErrorCode code);

		inline const bool HasError() const;
		
		inline const ErrorCode GetError() const;

		inline T& Get();

	private:
		Expected(ErrorCode code, ErrTAG) : m_Error(true) {
			m_ReturnVal.code = code;
		}
		
		union ReturnErrorUnion {
			T ret;
			ErrorCode code;
			~ReturnErrorUnion() {};
		} m_ReturnVal;

		bool m_Error = false;
	};

	namespace Exceptions {
		// ==== ENGINE EXCEPTIONS ====
		SMASHER_EXCEPTION(ExpectedHasError)

		// ==== LAYER EXCEPTIONS ====
		SMASHER_EXCEPTION(LayerNotFound)
		SMASHER_EXCEPTION(LayerDuplicate)
		SMASHER_EXCEPTION(LayerEntityNotFound)
		SMASHER_EXCEPTION(CannotRemoveBaseLayer)

		// ==== COMPONENT EXCEPTIONS ====
		SMASHER_EXCEPTION(ComponentInvalid)
		SMASHER_EXCEPTION(ComponentDowncastFailed)
		SMASHER_EXCEPTION(MissingComponentDependency)

		// ==== CAMERA EXCEPTIONS ====
		SMASHER_EXCEPTION(CameraTargetNotSet)

		// ==== ENTITY EXCEPTIONS ====
		SMASHER_EXCEPTION(EntityDuplicateComponent)
		SMASHER_EXCEPTION(EntityComponentNotFound)

		// ==== EVENT EXCEPTIONS ====
		SMASHER_EXCEPTION(EventHandleInvalid)

		// ==== RESOURCE EXCEPTIONS ====
		SMASHER_EXCEPTION(ResourceFailedToLoad)
		SMASHER_EXCEPTION(ResourceNotLoaded)
		SMASHER_EXCEPTION(ResourceAlreadyExists)
		SMASHER_EXCEPTION(ResourceInvalidNumPaths)

		// ==== PHYSICS EXCEPTIONS ====
		SMASHER_EXCEPTION(Box2DWorldAlreadyCreated)
		SMASHER_EXCEPTION(Box2DBodyIdInvalid)

		// ==== OPENGL EXCEPTIONS ====
		SMASHER_EXCEPTION(GLEWInitFailed)
	}
}

#include "Smasher/Exceptions.inl"