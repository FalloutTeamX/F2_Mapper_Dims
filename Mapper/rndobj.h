//---------------------------------------------------------------------------
#ifndef rndobjH
#define rndobjH

class CRandomObj
{
public:
   int *objPid;

   int GetObjectID();

   CRandomObj(String pidLine);
   ~CRandomObj();

private:
    int count;
};
//---------------------------------------------------------------------------
#endif

