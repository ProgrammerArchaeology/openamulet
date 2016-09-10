
#include <amulet/filefind.h>

int main(void)
{
	Am_Filename found = Am_Filename();

	std::cout << "==== Printing default path search list ====" << std::endl;
	std::cout << Am_Default_Data_Locator;
	std::cout << "====   Adding /bin and reprinting      ====" << std::endl;
	Am_Default_Data_Locator.Add_Search_Directory(Am_Filename("/bin"));
	std::cout << Am_Default_Data_Locator;
	std::cout << "====   Locating sh                     ====" << std::endl;
	Am_Filename sh = Am_Filename();
	bool ok = Am_Default_Data_Locator.Find_File("sh", sh);
	std::cout << " ok=" << ok << " sh=" << sh << std::endl;
	if(!(ok)) return 1;
	std::cout << "====   Adding a non-directory          ====" << std::endl;
	Am_Default_Data_Locator.Add_Search_Directory("tototruc");
	std::cout << "====   Searching a non existent file   ====" << std::endl;
	ok = Am_Default_Data_Locator.Find_File("bidibidibidi", found);
	std::cout << " ok=" << ok << std::endl;
	if(ok) return 2;
	std::cout << "==== Re-adding /bin and reprinting     ====" << std::endl;
	Am_Default_Data_Locator.Add_Search_Directory(Am_Filename("/bin"));
	std::cout << Am_Default_Data_Locator;
	std::cout << "====   Removing /bin and reprinting    ====" << std::endl;
	Am_Default_Data_Locator.Remove_Search_Directory("/bin");
	std::cout << Am_Default_Data_Locator;
	std::cout << "====   Re-removing /bin and reprinting ====" << std::endl;
	Am_Default_Data_Locator.Remove_Search_Directory(Am_Filename("/bin"));
	std::cout << Am_Default_Data_Locator;
	std::cout << "====   Locating sh but should fail...  ====" << std::endl;
	ok = Am_Default_Data_Locator.Find_File("sh", found);
	std::cout << " ok=" << ok << std::endl;
	if(ok) return 3;
	std::cout << "====   Locating found '" << sh << "'   ====" << std::endl;
	ok = Am_Default_Data_Locator.Find_File(sh, found);
	std::cout << " ok=" << ok << " found=" << found << " sh=" << sh << std::endl;
	if(!ok) return 4;

	return 0;
}

