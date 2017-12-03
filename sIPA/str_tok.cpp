#include <iostream>
#include <string.h>
#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
{
   char *buf;

   char *ipImage;
   char *opImage;
   char *oper;


   cout << argv[1] << endl;

   /*if ((buf = getenv("QUERY_STRING")) != NULL)
   {
      cout << "Args Found.\n" << endl;
      cout << buf << endl;
      ipImage = strtok(buf, "#");
      opImage = strtok(NULL, "#");
      oper = strtok(NULL, "#");
      cout << ipImage << endl;
      cout << opImage << endl;
      cout << oper << endl;
   }
   else
   {
      cout << "No Args Found.\n" << endl;
   }*/
   cout << "Good bye" << endl;
   return 0;
}
