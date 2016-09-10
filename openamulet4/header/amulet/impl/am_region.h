#ifndef AM_REGION_H
#define AM_REGION_H

class _OA_DL_CLASSIMPORT Am_Region
{
	public:
		//creators
		static  Am_Region* Create();

		virtual void Clear() = 0;
		virtual void Destroy() = 0;
		virtual bool In(int x, int y) = 0;
		virtual bool In(int left, int top, unsigned int width, unsigned int height, bool &total) = 0;
		virtual bool In(Am_Region *rgn, bool &total) = 0;
		virtual void Intersect(int left, int top, unsigned int width,unsigned int height) = 0;
		virtual void Set(int left, int top, unsigned int width,	unsigned int height) = 0;
		#ifdef POLYGONAL_REGIONS
		virtual void Set (Am_Point_List points) = 0;
		#endif
		virtual void Push(Am_Region* region) = 0;
		virtual void Push(int left, int top, unsigned int width, unsigned int height) = 0;
		virtual void Pop() = 0;
		virtual void Union(int left, int top, unsigned int width, unsigned int height) = 0;
		virtual ~Am_Region() { }
};

#endif // AM_REGION_H
