#ifndef __the_statics__
#define __the_statics__

bool idSortAscending;
bool lengthSortAscending;
struct sortStruct
{
  int id;
  double length;
};
int wxCALLBACK CompareItems( long item1, long item2, long sortData );

#endif // __the_statics__
