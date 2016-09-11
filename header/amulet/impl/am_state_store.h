#ifndef OPAL_STATE_STORE_H
#define OPAL_STATE_STORE_H

class Am_Object;
class Am_Drawonable;

class _OA_DL_CLASSIMPORT Am_State_Store
{
	public:
	Am_State_Store(Am_Object self, Am_Object owner, bool visible, int left,
		int top, int width, int height);
	void Add(bool needs_update);
		void Remove();
	static Am_State_Store* Narrow(Am_Ptr ptr)
	{ return (Am_State_Store*)ptr; 
	}

	bool Visible(Am_Drawonable* drawonable, int x_offset, int y_offset);

	static void Invoke();
	static void Shutdown()
	{ shutdown = true; 
	}

	private:
	Am_Object owner;
	bool visible;
	int left;
	int top;
	int width;
	int height;

	bool in_list;
	bool needs_update;
	Am_Object self;
	Am_State_Store* next;

	void Invalidate();

	static Am_State_Store* invalidation_list;
	static bool shutdown;
};

#endif /* OPAL_STATE_STORE_H */
