
class BgObject{
	pid_t pid;
	std::string workingDir;
	char status;
public:
	BgObject();
	BgObject(pid_t inPid, std::string inDir, char inStat);
	pid_t getPid(){ return pid; }
	std::string getWorkingDir() { return workingDir; }
	char getStatus() { return status; }
	void setPid(pid_t inPid) { pid = inPid; }
	void setWorkingDir(std::string inDir) { workingDir = inDir; }
	void setStatus(char inStat) { status = inStat; }
};

BgObject::BgObject(){
	pid = 0;
	workingDir = "";
	status = 'R';
}

BgObject::BgObject(pid_t inPid, std::string inDir, char inStat){
	pid = inPid;
	workingDir = inDir;
	status = inStat; 
}