#include "pch.h"
#include "KeyBoard.h"

KeyBoard* KeyBoard::s_instance = nullptr;

void KeyBoard::Init()
{
	if (!s_instance) s_instance = new KeyBoard();
}

void KeyBoard::Destroy()
{
	if (s_instance)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

bool KeyBoard::IsValid()
{
	return s_instance;
}

KeyBoard& KeyBoard::Get()
{
	return *s_instance;
}

void KeyBoard::Update()
{
	m_keyState1 = m_keyState0;
	for (auto& state : m_keyState0)
	{
		state.second = state.second & KeyState::held;
	}
}

void KeyBoard::SetKey(Key key, KeyState state)
{
	m_keyState1[(int)key] = state;
}

bool KeyBoard::GetKey(Key key, KeyState state) const
{
	if (!m_keyState1.contains((int)key) || state == KeyState::invalid)
	{
		return false;
	}
	else
	{
		return (m_keyState1.at((int)key) & state) == state;
	}
}
