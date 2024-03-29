#include "Header.hpp"


std::vector<std::string>			split(std::string str, std::string separator)
{
	std::vector<std::string> tab;
	std::string temp;

	for (unsigned int i = 0; i < str.size(); i++)
	{
		if (separator.find(str[i]) == SIZE_MAX)
			temp.push_back(str[i]);
		else{
			if (!temp.empty()){
				tab.push_back(temp);
				temp.clear();
			}
		}
	}
	if (!temp.empty())
		tab.push_back(temp);
	return (tab);
}
int									searchInVec(std::string find, std::vector<std::string> entry){
	for (size_t i = 0; i < entry.size(); i++){
		if (!find.compare(entry[i]))
			return (i);
	}
	return (-1);
}
std::string							fileToString(std::string file){
	std::string fileToString;
	std::string line;
std::ifstream	ifs(file.c_str());

	if (ifs.fail()){
		std::cerr << "Reading Error 2" << std::endl;
		return (fileToString);
	}
	while (std::getline(ifs, line)){
		fileToString.append(line);
	}
	(ifs).close();
	return (fileToString);
}
std::string							convertInSpaces(std::string line){
	std::string symbols = "\t\n\v\r\f";
	for (unsigned int i = 0; i < line.size(); i++){
		for (unsigned int j = 0; j < symbols.size(); j++){
			if (line[i] == symbols[j])
				line[i] = ' ';
		}
	}
	return (line);
}
std::string							cleanLine(std::string    &line){
	std::string lineCleaned = convertInSpaces(line);
	std::vector<std::string> res = split(lineCleaned, " ");

	lineCleaned.clear();
	for (unsigned int i = 0; i < res.size(); i++){
		lineCleaned.append(res[i]);
		if (i < res.size() - 1)
			lineCleaned.append(" ");
	}
	if (lineCleaned[lineCleaned.size() -1] == ';'){
		lineCleaned.resize(lineCleaned.size() - 1);
	}
	return (lineCleaned);
}
int									fileIsOpenable(std::string path){
	std::ifstream opfile;

	opfile.open(path.data());
	if (!opfile.is_open())
		return (0);
	opfile.close();
	return (1);
}
bool								folderIsOpenable(std::string repos){
	DIR		*folder = opendir((repos).c_str());
	bool	ret = false;
	if(folder) {
		closedir(folder);
		ret = true;
	}
	return (ret);
}
std::vector<std::string>			listFilesInFolder(std::string repos){
	struct dirent				*entry;
	DIR							*folder;
	std::vector<std::string>	ret;

	folder = opendir(repos.c_str());
	while (folder && (entry = readdir(folder))) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
			ret.push_back(entry->d_name);
		} 
	}
	closedir(folder);
	return (ret);
}
char 								**mergeArrays(char **tab1, char **tab2, int freeOption){
	int length = 0;
	char **newTab;
	for (unsigned int i = 0; tab1 && tab1[i]; i++)
		length++;
	for (unsigned int j = 0; tab2 && tab2[j]; j++)
		length++;
	if (!(newTab = (char**)malloc(sizeof(char*) * (length + 1))) || length == 0)
		return (NULL);
	length = 0;
	for (unsigned int i = 0; tab1 && tab1[i]; i++)
		newTab[length++] = tab1[i];
	for (unsigned int j = 0; tab2 && tab2[j]; j++)
		newTab[length++] = tab2[j];
	newTab[length] = NULL;
	if (freeOption == 1)
		free(tab1);
	else if (freeOption == 2)
		free(tab2);
	else if (freeOption == 3){
		free(tab1);
		free(tab2);
	}
	return (newTab);
}
bool 								inArray(std::string arr[], std::string needle){ 
	for (size_t i = 0; i < arr->size(); i++)
		if (arr[i] == needle)
			return (true);
	return (false);
}
std::string							getTime(){
	time_t ttime = time(0);
	std::string ret = std::string(ctime(&ttime));
	ret.erase(ret.size() - 1);
	return (ret);
}
long long unsigned int				getSizeFileBits(std::string filename) {
	std::ifstream opfile;
	long long unsigned int ret = 0;
	
	opfile.open(filename.c_str(), std::ios::in);
	if (!opfile.is_open())
		return (0);
	opfile.seekg(0, std::ios::end);
	ret = opfile.tellg();
	opfile.close();
	return (ret);
}
std::string							NumberToString(long int Number)
{
	std::ostringstream ss;
    ss << Number;
    return ss.str();
}
std::string							replaceStr(std::string line, std::string s1, std::string s2)
{
    size_t i = 0;

    while ((i = line.find(s1, i)) < line.length())
    {
         line.replace(i, s1.length(), s2);
         i++;
    }
    return line;
}
std::string							replaceStrStart(std::string line, std::string s1, std::string s2)
{
    if(line.find(s1, 0) == 0)
    {
         line.replace(0, s1.length() - 1, s2);
    }
    return line;
}
std::string							getfilename(std::string uri) {
	std::string ret;
	ret = &uri[uri.find_last_of("/") + 1];
	return (ret);
}

std::string							CleanBody(std::string request, std::string contentType){
	std::string body;

	if (request.find("\r\n\r\n") != request.rfind("\r\n\r\n") || (request.find("\r\n\r\n") != request.rfind("\n") && request.rfind("\n") != SIZE_MAX )){
		if (contentType.find("boundary=") != SIZE_MAX && contentType.find("multipart/form-data;") != SIZE_MAX) {
			body = request.substr(request.find("\r\n\r\n") + 2, request.rfind("\r\n\r\n"));
		} else if (request.find("\r\n\r\n") != request.rfind("\r\n\r\n")){
			body = request.substr(request.find("\r\n\r\n") + 2, request.rfind("\r\n\r\n"));
			std::vector<std::string> bodyvec = split(body, "\r\n");
			body = "";
			for (size_t i = 1; i < bodyvec.size(); i += 2){
				body += bodyvec[i];
			}
		}
		else
			body = request.substr(request.find("\r\n\r\n") + 4, request.rfind("\n"));
	}
	return (body);
}
std::string decode64(const std::string &in)
{

	std::string out;

	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++)
		T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

	int val = 0, valb = -8;
	for (size_t i = 0; in[i]; i++)
	{
		if (T[in[i]] == -1)
			break;
		val = (val << 6) + T[in[i]];
		valb += 6;
		if (valb >= 0)
		{
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}