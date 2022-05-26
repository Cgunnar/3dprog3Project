#pragma once

enum struct KeyState
{
	invalid = 0,
	clicked = 1,
	held = 2,
	released = 4,
};

enum struct Key
{
	A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	_0 = 0x30, _1, _2, _3, _4, _5, _6, _7, _8, _9,
	_0np = 0x60, _1np, _2np, _3np, _4np, _5np, _6np, _7np, _8np, _9np,
	f1 = 0x70, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
	lShift = 0xA0, rShift, lCtrl, rCtrl,
	back = 0x08, tab,
	enter = 0x0D,
	capsLock = 0x14,
	esc = 0x1B,
	space = 0x20,
	left = 0x25, up, right, down,
};

inline KeyState operator &(KeyState l, KeyState r)
{
	return (KeyState)((int)l & (int)r);
}
inline KeyState operator |(KeyState l, KeyState r)
{
	return (KeyState)((int)l | (int)r);
}

class Window;
class KeyBoard
{
	friend Window;
public:
	static void Init();
	static void Destroy();
	static bool IsValid();
	static KeyBoard& Get();
	void Update();
	void SetKey(Key key, KeyState state);
	bool GetKey(Key key, KeyState state) const;
private:
	KeyBoard() = default;
	~KeyBoard() = default;
	KeyBoard(const KeyBoard& other) = delete;
	KeyBoard& operator=(const KeyBoard& other) = delete;



	static KeyBoard* s_instance;
	std::unordered_map<int, KeyState> m_keyState0;
	std::unordered_map<int, KeyState> m_keyState1;
};

