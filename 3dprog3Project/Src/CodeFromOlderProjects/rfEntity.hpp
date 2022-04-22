/*MIT License

Copyright(c) 2022 Gunnar Cerne

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

#include <unordered_map>
#include <queue>
#include <vector>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <assert.h>

#ifdef _DEBUG
#include <iostream>
#include <string>
#include <debugapi.h>
#endif // _DEBUG


namespace rfe
{
	using ComponentTypeID = size_t;
	using ComponentIndex = size_t;
	using EntityID = size_t;

	class EntityComponentManager;
	class EntityReg;
	class ECSSerializer;
	constexpr double timeStep = 1.0 / 100.0;

	class Entity
	{
		friend EntityComponentManager;
	public:
		Entity() : m_entityIndex(-1), m_entCompManRef(nullptr) {}
		~Entity();
		Entity(const Entity& other);
		Entity& operator=(const Entity& other);
		Entity(Entity&& other) noexcept;
		Entity& operator=(Entity&& other) noexcept;

		template<typename T>
		T* GetComponent();

		template<typename T>
		requires std::is_copy_assignable_v<T>
			T* AddComponent(const T& component) const;

		template<typename T, typename... Args> requires std::is_copy_assignable_v<T>
		T* AddComponent(Args&&... args);

		template<typename T>
		void RemoveComponent() const;

		void Reset();
		EntityID GetID() const { return m_entityIndex; }
		int GetRefCount() const { return s_refCounts[m_entityIndex]; }
		bool Empty() const
		{
#ifdef DEBUG
			if (m_entityIndex != -1)
			{
				assert(s_refCounts[m_entityIndex] > 0);
				assert(m_entCompManRef);
			}
			else
			{
				assert(!m_entCompManRef);
			}

#endif // DEBUG


			return m_entityIndex == -1 || !m_entCompManRef || s_refCounts[m_entityIndex] <= 0;
		}
		operator const EntityID() const { return m_entityIndex; }

	private:
		Entity(EntityID ID, EntityComponentManager* entReg);

		EntityID m_entityIndex;
		EntityComponentManager* m_entCompManRef;
		inline static std::vector<int> s_refCounts;
	};


	class BaseComponent
	{
		friend EntityComponentManager;
		friend ECSSerializer;

	public:
		EntityID GetEntityID() const { return entityIndex; }
		Entity GetEntity() const;
	protected:
		EntityID entityIndex = -1;

		static ComponentTypeID registerComponent(size_t size, std::string name, std::function<ComponentIndex(BaseComponent*)> createFunc,
			std::function<BaseComponent* (ComponentIndex)> fetchFunc, std::function<EntityID(ComponentIndex)> deleteFunc,
			std::function<void(size_t)> resize, std::function<char* ()> getArray, std::function<size_t()> count, std::function<void(ComponentIndex)> destroy);
	private:
		struct ComponentUtility
		{
			size_t size = 0;
			std::string name;
			std::function<ComponentIndex(BaseComponent*)> createComponent;
			std::function<BaseComponent* (ComponentIndex)> fetchComponentAsBase;
			std::function<EntityID(ComponentIndex)> deleteComponent;
			std::function<void(size_t)> resizeArrayT;
			std::function<char* ()> getArrayPointer;
			std::function<size_t()> componentCount;
			std::function<void(ComponentIndex)> compDestroy;
		};

		inline static std::vector<ComponentUtility> s_componentRegister;

		static size_t getSize(ComponentTypeID id);
		static std::function<ComponentIndex(BaseComponent*)> getCreateComponentFunction(ComponentTypeID id);
		static ComponentUtility getComponentUtility(ComponentTypeID id);
	};

	struct ComponentMetaData
	{
		ComponentMetaData(ComponentTypeID typeID, ComponentIndex index, EntityID entIndex) : typeID(typeID), index(index), entityIndex(entIndex) {}
		ComponentTypeID typeID;
		ComponentIndex index;
		EntityID entityIndex;
	};


	template<typename T>
	class Component : public BaseComponent
	{
		friend EntityComponentManager;
		friend EntityReg;

	private:
		using BaseComponent::entityIndex;
		using BaseComponent::registerComponent;

	public:
		static const ComponentTypeID typeID;
		static const size_t size;
		static const std::string componentName;

	private:
		static std::vector<T> componentArray;

		void destroy()
		{

		}

		template<typename T>
		static void destroy(ComponentIndex index)
		{
			componentArray[index].destroy();
		}

		//requires std::is_trivially_copy_assignable_v<T>
		template<typename T>
		requires std::is_copy_assignable_v<T>
			static ComponentIndex createComponent(BaseComponent* comp)
		{
			componentArray.push_back(*static_cast<T*>(comp));
			return componentArray.size() - 1;
		}
		template<typename T>
		static BaseComponent* fetchComponent(ComponentIndex index)
		{
			return static_cast<BaseComponent*>(&componentArray[index]);
		}
		template<typename T>
		static EntityID deleteComponent(ComponentIndex index)
		{
			EntityID entityOwningBackComponent = -1;
			if (index + 1 != componentArray.size()) // last component skip this step
			{
				componentArray[index] = componentArray.back();
				entityOwningBackComponent = componentArray[index].entityIndex;
			}
			componentArray.pop_back();
			return entityOwningBackComponent;
		}
		static void resizeComponentArray(size_t elements) { componentArray.resize(elements); }
		static char* getArrayStartPointer() { return reinterpret_cast<char*>(componentArray.data()); }
		static size_t getCount() { return componentArray.size(); }
	};

	template<typename T>
	std::vector<T> Component<T>::componentArray;

	template<typename T>
	const ComponentTypeID Component<T>::typeID = BaseComponent::registerComponent(sizeof(T), typeid(T).name(), Component<T>::createComponent<T>,
		Component<T>::fetchComponent<T>, Component<T>::deleteComponent<T>, Component<T>::resizeComponentArray, Component<T>::getArrayStartPointer,
		Component<T>::getCount, Component<T>::destroy<T>);

	template<typename T>
	const size_t Component<T>::size = sizeof(T);

	template<typename T>
	const std::string Component<T>::componentName = typeid(T).name();




	class EntityComponentManager
	{
		friend EntityReg;
		friend ECSSerializer;
		friend BaseComponent;
		EntityComponentManager() = default;
		EntityComponentManager(const EntityComponentManager&) = delete;
		EntityComponentManager& operator=(const EntityComponentManager&) = delete;
		~EntityComponentManager();

	public:
		Entity& CreateEntity();
		void RemoveEntity(Entity& entity); // this should be encapsulated better
		void AddComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component);

		template<typename T, typename... Args> requires std::is_copy_assignable_v<T>
		T* AddComponent(EntityID entityID, Args&&... args);

		template<typename T> requires std::is_copy_assignable_v<T>
		T* AddComponent(EntityID entityID, const T& component);

		template<typename T>
		void RemoveComponent(EntityID entityID);

		template<typename T>
		T* GetComponent(EntityID entityID);

	private:
		template<typename... T>
		void RunScripts(float dt);

		template<typename T>
		void RunScript(float dt);

		template<typename... T>
		void StartScripts();

		template<typename T>
		void StartScript();

		void RemoveComponent(ComponentTypeID type, EntityID entityID);
		void RemoveInternalEntity(Entity& entity);
		Entity CreateEntityFromExistingID(EntityID id) { return Entity(id, this); }

	private:
		std::vector<std::vector<ComponentMetaData>> m_entitiesComponentHandles;
		std::queue<EntityID> m_freeEntitySlots;
		std::vector<Entity> m_entityRegistry;
		bool clearHasBeenCalled = false;
	};


	class EntityReg
	{
		friend ECSSerializer;
		friend BaseComponent;
		EntityReg() = delete;
	public:
		static void Clear();

		template<typename... Args>
		static void RunScripts(float dt);

		template<typename... Args>
		static void StartScripts();

		static Entity& CreateEntity();
		//static void removeEntity(Entity& entity);
		static void AddComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component);
		static const std::vector<Entity>& getAllEntities();

		template<typename T>
		requires std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T>
		static T* AddComponent(EntityID entityID, const T& component);

		template<typename T, typename... Args> requires std::is_copy_assignable_v<T>
		static T* AddComponent(EntityID entityID, Args&&... args);

		template<typename T>
		static void RemoveComponent(EntityID entityID);

		template<typename T>
		static T* GetComponent(EntityID ID);

		template<typename T>
		static std::vector<T>& GetComponentArray();

		template<typename T, typename ... Args>
		static std::vector<Entity> ViewEntities();

		
	private:
		template<typename T>
		static bool ViewEntitiesHelper(Entity& entity);
		inline static EntityComponentManager m_entCompManInstance;
	};

	//EntityReg definitions
	//----------------------------------------------------

	inline Entity& EntityReg::CreateEntity()
	{
		return m_entCompManInstance.CreateEntity();
	}

	inline void EntityReg::Clear()
	{
		// this will delete all entities and components, before componentesArray gets destroyed
		for (auto& e : m_entCompManInstance.m_entityRegistry)
		{
			if (!e.Empty())
			{
				if (e.GetRefCount() != 1)
				{
					OutputDebugString(L"[ERROR] Release all outstanding references to entities before calling EntityReg::clear(). \n\tEntityID: ");
					OutputDebugString(std::to_wstring(e.GetID()).c_str());
					OutputDebugString(L"\n\tNumRefs needed to be released: ");
					OutputDebugString(std::to_wstring(e.GetRefCount() - 1).c_str());
					OutputDebugString(L"\n");
					throw std::runtime_error("release all outstanding references to entities before calling EntityReg::clear().");
				}
				m_entCompManInstance.RemoveEntity(e);
			}
		}
		m_entCompManInstance.m_entityRegistry.clear();
		m_entCompManInstance.clearHasBeenCalled = true;
	}


	template<typename ...Args>
	inline void EntityReg::RunScripts(float dt)
	{
		m_entCompManInstance.RunScripts<Args...>(dt);
	}

	template<typename ...Args>
	inline void EntityReg::StartScripts()
	{
		m_entCompManInstance.StartScripts<Args...>();
	}

	inline void EntityReg::AddComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component)
	{
		m_entCompManInstance.AddComponent(entityID, typeID, component);
	}
	
	template<typename T>
	requires std::is_copy_assignable_v<T>&& std::is_copy_constructible_v<T>
	inline T* EntityReg::AddComponent(EntityID entityID, const T& component)
	{
		return m_entCompManInstance.AddComponent<T>(entityID, component);
	}

	template<typename T, typename ...Args>requires std::is_copy_assignable_v<T>
	inline T* EntityReg::AddComponent(EntityID entityID, Args && ...args)
	{
		return m_entCompManInstance.AddComponent<T>(entityID, std::forward<Args>(args)...);
	}

	template<typename T>
	inline void EntityReg::RemoveComponent(EntityID entityID)
	{
		m_entCompManInstance.RemoveComponent<T>(entityID);
	}

	template<typename T>
	inline T* EntityReg::GetComponent(EntityID ID)
	{
		return m_entCompManInstance.GetComponent<T>(ID);
	}

	template<typename T>
	inline std::vector<T>& EntityReg::GetComponentArray()
	{
		return T::componentArray;
	}

	template<typename T, typename ...Args>
	inline std::vector<Entity> EntityReg::ViewEntities()
	{
		std::vector<Entity> entities;
		entities.reserve(T::componentArray.size());
		for (auto& comp : T::componentArray)
		{
			Entity entity = m_entCompManInstance.m_entityRegistry[comp.entityIndex];
			bool hasComponent = true;
			int e[]{ 0, (hasComponent = hasComponent && ViewEntitiesHelper<Args>(entity), 0)... };
			if (hasComponent)
			{
				entities.push_back(entity);
			}
		}
		return entities;
	}

	template<typename T>
	inline bool EntityReg::ViewEntitiesHelper(Entity& entity)
	{
		return entity.GetComponent<T>() != nullptr;
	}

	inline const std::vector<Entity>& EntityReg::getAllEntities()
	{
		return m_entCompManInstance.m_entityRegistry;
	}



	//Entity definitions
	//-------------------------------------
	inline Entity::Entity(EntityID ID, EntityComponentManager* entReg) : m_entityIndex(ID), m_entCompManRef(entReg)
	{
		if (ID >= s_refCounts.size())
		{
			s_refCounts.push_back(1);
		}
		else
		{
			s_refCounts[ID]++;
		}
	}

	inline Entity::Entity(const Entity& other)
	{
		if (!other.Empty())
		{
			this->m_entCompManRef = other.m_entCompManRef;
			this->m_entityIndex = other.m_entityIndex;
			s_refCounts[m_entityIndex]++;
		}
		else
		{
			this->m_entCompManRef = nullptr;
			this->m_entityIndex = -1;
		}
	}

	inline Entity::~Entity()
	{

//#ifdef _DEBUG
//		if (!this->Empty())
//		{
//			OutputDebugString(L"~Entity\tindex: ");
//			OutputDebugString(std::to_wstring(m_entityIndex).c_str());
//			OutputDebugString(L", refCount: ");
//			OutputDebugString(std::to_wstring(s_refCounts[m_entityIndex]).c_str());
//			OutputDebugString(L"\n");
//		}
//#endif // _DEBUG
		this->Reset();
	}

	inline Entity& Entity::operator=(const Entity& other)
	{
		this->Reset();
		if (!other.Empty())
		{
			this->m_entCompManRef = other.m_entCompManRef;
			this->m_entityIndex = other.m_entityIndex;
			s_refCounts[m_entityIndex]++;
		}
		else
		{
			this->m_entCompManRef = nullptr;
			this->m_entityIndex = -1;
		}
		return *this;
	}

	inline Entity::Entity(Entity&& other) noexcept
	{
		this->m_entityIndex = other.m_entityIndex;
		this->m_entCompManRef = other.m_entCompManRef;
		//invalidate other
		other.m_entCompManRef = nullptr;
		other.m_entityIndex = -1;
	}

	inline Entity& Entity::operator=(Entity&& other) noexcept
	{
		this->Reset();
		this->m_entityIndex = other.m_entityIndex;
		this->m_entCompManRef = other.m_entCompManRef;
		//invalidate other
		other.m_entCompManRef = nullptr;
		other.m_entityIndex = -1;
		return *this;
	}
	inline void Entity::Reset()
	{
		if (Empty()) return;
		if (s_refCounts[m_entityIndex] <= 2)
		{
			m_entCompManRef->RemoveEntity(*this);
			assert(this->Empty()); //removeEntity take care of reseting the entity
		}
		else
		{
			s_refCounts[m_entityIndex]--;
			m_entityIndex = -1;
			m_entCompManRef = nullptr;
		}
	};

	template<typename T>
	inline T* Entity::GetComponent()
	{
		return m_entCompManRef->GetComponent<T>(this->m_entityIndex);
	}

	template<typename T> requires std::is_copy_assignable_v<T>
	inline T* Entity::AddComponent(const T& component) const
	{
		return m_entCompManRef->AddComponent<T>(this->m_entityIndex, component);
	}

	template<typename T, typename ...Args> requires std::is_copy_assignable_v<T>
	inline T* Entity::AddComponent(Args && ...args)
	{
		return m_entCompManRef->AddComponent<T>(this->m_entityIndex, std::forward<Args>(args)...);
	}

	template<typename T>
	inline void Entity::RemoveComponent() const
	{
		m_entCompManRef->RemoveComponent<T>(this->m_entityIndex);
	}


	//EntityComponentManager definitions
	//----------------------------------------------------

	inline EntityComponentManager::~EntityComponentManager()
	{
		if (!clearHasBeenCalled)
		{
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			assert(false);
		}
	}

	inline Entity& EntityComponentManager::CreateEntity()
	{
		EntityID index;
		if (!m_freeEntitySlots.empty())
		{
			index = m_freeEntitySlots.front();
			m_freeEntitySlots.pop();
			m_entitiesComponentHandles[index].clear();
			m_entityRegistry[index] = std::move(Entity(index, this));
		}
		else
		{
			index = m_entitiesComponentHandles.size();
			m_entitiesComponentHandles.emplace_back(std::vector<ComponentMetaData>());
			m_entityRegistry.emplace_back(Entity(index, this));
			assert(m_entityRegistry.back().GetID() == m_entityRegistry[index].GetID());
		}
		return m_entityRegistry[index];
	}

	inline void EntityComponentManager::RemoveInternalEntity(Entity& entity)
	{
		assert(!entity.Empty());
		assert(entity.s_refCounts[entity.m_entityIndex] == 1);
		assert(m_entitiesComponentHandles.size() == m_entityRegistry.size());
		assert(&entity == &m_entityRegistry[entity.m_entityIndex]);

		auto& components = m_entitiesComponentHandles[entity.m_entityIndex];
		for (auto& c : components)
		{
			auto compUtil = BaseComponent::getComponentUtility(c.typeID);
			compUtil.compDestroy(c.index);

			RemoveComponent(c.typeID, entity.m_entityIndex);
		}
		if (entity.m_entityIndex + 1 == m_entitiesComponentHandles.size())
		{
			m_entitiesComponentHandles.pop_back();
		}
		else
		{
			m_entitiesComponentHandles[entity.m_entityIndex].clear();
			m_freeEntitySlots.push(entity.m_entityIndex);
		}

		//reset
		entity.s_refCounts[entity.m_entityIndex] = 0;
		entity.m_entCompManRef = nullptr;
		entity.m_entityIndex = -1;
	}

	inline void EntityComponentManager::RemoveEntity(Entity& entity)
	{
		assert(!entity.Empty());

#ifdef DEBUG
		std::string debugOut = "Removed Entity: " + std::to_string(entity.m_entityIndex) + ", refCount: "
			+ std::to_string(entity.s_refCounts[entity.m_entityIndex]) + "\n";
		std::cout << debugOut;
#endif // DEBUG

		if (entity.s_refCounts[entity.m_entityIndex] > 2)
		{

			std::string debugOut = "[ERROR] release " + std::to_string(entity.s_refCounts[entity.m_entityIndex] - 2) +
				" references to Entity, ID: " + std::to_string(entity.m_entityIndex) +
				"\n\tOnly two references is allowd when removeEntity(Entity) is called.\n\tOne internal and one for the argument to the call.\n";
#ifdef DEBUG
			std::cout << debugOut;
#endif // DEBUG



			throw std::runtime_error(debugOut);
		}


		if (entity.GetRefCount() == 1)
		{
			RemoveInternalEntity(entity);
		}
		else if (entity.GetRefCount() == 2)
		{
			EntityID id = entity.GetID();

			//first reset entity: ref count will now be 1, can't call entity.reset(), because that will lead to recursion
			entity.m_entCompManRef = nullptr;
			entity.s_refCounts[entity.m_entityIndex]--;
			entity.m_entityIndex = -1;

			if (id + 1 == m_entitiesComponentHandles.size())
			{
				m_entityRegistry.pop_back(); //second reset internal entity
			}
			else
			{
				m_entityRegistry[id].Reset(); //second reset internal entity
			}
			assert(entity.s_refCounts[id] == 0);
		}
	}

	inline void EntityComponentManager::AddComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component)
	{
		auto createFunc = BaseComponent::getCreateComponentFunction(typeID);
		ComponentIndex index = createFunc(component);
		m_entitiesComponentHandles[entityID].emplace_back(typeID, index, entityID);
	}


	template<typename T, typename ...Args> requires std::is_copy_assignable_v<T>
	inline T* EntityComponentManager::AddComponent(EntityID entityID, Args&& ...args)
	{
		ComponentTypeID typeID = T::typeID;
		ComponentIndex index;
		index = T::componentArray.size();
		T::componentArray.emplace_back(std::forward<Args>(args)...);

		T* compPtr = &T::componentArray[index];
		compPtr->entityIndex = entityID;
		m_entitiesComponentHandles[entityID].emplace_back(typeID, index, entityID);
		return compPtr;
	}

	template<typename T> requires std::is_copy_assignable_v<T>
	inline T* EntityComponentManager::AddComponent(EntityID entityID, const T& comp)
	{
		ComponentTypeID typeID = T::typeID;
		ComponentIndex index;
		index = comp.componentArray.size();
		T::componentArray.emplace_back(comp);

		T* compPtr = &T::componentArray[index];
		compPtr->entityIndex = entityID;
		m_entitiesComponentHandles[entityID].emplace_back(typeID, index, entityID);
		return compPtr;
	}

	template<typename T>
	inline void EntityComponentManager::RemoveComponent(EntityID entityID)
	{
		RemoveComponent(T::typeID, entityID);
	}

	template<typename T>
	class NativeScriptComponent : public Component<T>
	{
	public:
		bool m_hasStarted = false;

		//helpers functions
		template<typename T>
		T* GetComponent()
		{
			return EntityReg::GetComponent<T>(this->GetEntityID());
		}
		template<typename T, typename... Args>
		T* AddComponent(Args&& ...args)
		{
			return EntityReg::AddComponent<T>(this->GetEntityID(), std::forward<Args>(args)...);
		}
		template<typename T>
		void RemoveComponent()
		{
			EntityReg::RemoveComponent<T>(this->GetEntityID());
		}

		void OnStart() {};

		//On update functions
		void OnUpdate(float dt) {};
		void OnFixedUpdate(float dt) {};
	};

	template<typename... T>
	inline void EntityComponentManager::RunScripts(float dt)
	{
		int e[]{ 0, (RunScript<T>(dt), 0)... };
	}

	template<typename T>
	inline void EntityComponentManager::RunScript(float dt)
	{
		for (auto& script : T::componentArray)
		{
			if (!script.m_hasStarted)
			{
				script.OnStart();
				script.m_hasStarted = true;
			}
			script.OnUpdate(dt);
		}
		static float deltaTime = 0;
		deltaTime += dt;
		while (timeStep < deltaTime)
		{
			for (auto& script : T::componentArray)
			{
				script.OnFixedUpdate(static_cast<float>(timeStep));
			}
			deltaTime -= timeStep;
		}
		
	}

	template<typename ...T>
	inline void EntityComponentManager::StartScripts()
	{
		int e[]{ 0, (StartScript<T>(), 0)...};
	}

	template<typename T>
	inline void EntityComponentManager::StartScript()
	{
		for (auto& script : T::componentArray)
		{
			assert(!script.m_hasStarted);
			script.OnStart();
			script.m_hasStarted = true;
		}
	}

	inline void EntityComponentManager::RemoveComponent(ComponentTypeID type, EntityID entityID)
	{
		auto& entityComponentHandles = m_entitiesComponentHandles[entityID];
		if (auto iti = std::ranges::find_if(entityComponentHandles.begin(), entityComponentHandles.end(),
			[type](ComponentMetaData c) { return type == c.typeID; });
			iti != entityComponentHandles.end())
		{
			auto componentUtilityFunctions = BaseComponent::getComponentUtility(iti->typeID);
			if (EntityID movedCompEntity = componentUtilityFunctions.deleteComponent(iti->index); movedCompEntity != -1)
			{
				auto& movedCompEntityHandles = m_entitiesComponentHandles[movedCompEntity];
				if (auto itj = std::ranges::find_if(movedCompEntityHandles.begin(), movedCompEntityHandles.end(),
					[type](ComponentMetaData c) { return type == c.typeID; });
					itj != movedCompEntityHandles.end())
				{
					itj->index = iti->index;
				}
				else
				{
					assert(false); // find_if above should not fail
				}
			}
			*iti = entityComponentHandles.back();
			entityComponentHandles.pop_back();
		}
	}

	template<typename T>
	inline T* EntityComponentManager::GetComponent(EntityID entityID)
	{
		auto& entityComponents = m_entitiesComponentHandles[entityID];
		if (auto it = std::ranges::find_if(entityComponents.begin(), entityComponents.end(),
			[](ComponentMetaData c) { return T::typeID == c.typeID; });
			it != entityComponents.end())
		{
			auto r = &T::componentArray[it->index];
			return r;
		}
		return nullptr;
	}


	//BaseCompoent definitions
	//---------------------------------------------------
	inline ComponentTypeID BaseComponent::registerComponent(size_t size, std::string name, std::function<ComponentIndex(BaseComponent*)> createFunc,
		std::function<BaseComponent* (ComponentIndex)> fetchFunc, std::function<EntityID(ComponentIndex)> deleteFunc,
		std::function<void(size_t)> resize, std::function<char* ()> getArray, std::function<size_t()> count, std::function<void(ComponentIndex)> destroy)
	{
		ComponentTypeID compID = s_componentRegister.size();
		ComponentUtility comUtil;
		comUtil.size = size;
		comUtil.name = std::move(name);
		comUtil.createComponent = createFunc;
		comUtil.fetchComponentAsBase = fetchFunc;
		comUtil.deleteComponent = deleteFunc;
		comUtil.getArrayPointer = getArray;
		comUtil.resizeArrayT = resize;
		comUtil.componentCount = count;
		comUtil.compDestroy = destroy;
		s_componentRegister.push_back(comUtil);
		return compID;
	}

	inline Entity BaseComponent::GetEntity() const
	{
		return EntityReg::m_entCompManInstance.CreateEntityFromExistingID(entityIndex);
	}

	inline size_t BaseComponent::getSize(ComponentTypeID id)
	{
		return s_componentRegister[id].size;
	}
	inline std::function<ComponentIndex(BaseComponent*)> BaseComponent::getCreateComponentFunction(ComponentTypeID id)
	{
		return s_componentRegister[id].createComponent;
	}
	inline BaseComponent::ComponentUtility BaseComponent::getComponentUtility(ComponentTypeID id)
	{
		return s_componentRegister[id];
	}
}
