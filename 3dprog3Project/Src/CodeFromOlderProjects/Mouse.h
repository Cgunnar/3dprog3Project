#pragma once

struct MouseState
{
	int x = 0;
	int y = 0;
	int z = 0;

	float mouseCof = 0.001f;
	float deltaX = 0;
	float deltaY = 0;
	float deltaZ = 0;

	bool LMBClicked = false;
	bool LMBReleased = false;
	bool LMBHeld = false;

	bool RMBClicked = false;
	bool RMBReleased = false;
	bool RMBHeld = false;

	bool MMBClicked = false;
	bool MMBReleased = false;
	bool MMBHeld = false;
};

class Window;
class Mouse
{
	friend Window;
public:
	Mouse(const Mouse& other) = delete;
	Mouse& operator=(const Mouse& other) = delete;
	static void Init(HWND hwnd);
	static void Destroy();
	static Mouse& Get();
	void Update();
	MouseState State() const;

	enum class Mode
	{
		Visible = 1,
		Confined = 2
	};
	void SetMode(Mode mode);
	Mode GetMode() const;
private:
	Mode m_mode = Mode::Visible;
	static Mouse* s_mouseInstance;
	HWND m_hWnd;

	MouseState m_mouseState0{};
	MouseState m_mouseState1{};

	bool m_showCursor = false;
	bool m_windowOutOfFocus = false;

	Mouse(HWND hwnd);
};

inline Mouse::Mode operator ~(Mouse::Mode m)
{
	return (Mouse::Mode)~(int)m;
}
inline Mouse::Mode operator &(Mouse::Mode l, Mouse::Mode r)
{
	return (Mouse::Mode)((int)l & (int)r);
}
inline Mouse::Mode operator |(Mouse::Mode l, Mouse::Mode r)
{
	return (Mouse::Mode)((int)l | (int)r);
}