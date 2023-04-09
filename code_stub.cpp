//Disk File System simulation
//Authored by Guy Cohen
//206463606
//Disk Management System - is the way in which file names and their contents are located and organized on the hard disk. Without the file system, the information stored will not be organized into individual files and it will be impossible to locate and access their contents.
//The file's parts are saved on small units - the blocks.
//The function of the disk management system is to map all the parts of the file that are stored on the disk.
#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <fcntl.h>


using namespace std;

#define DISK_SIZE 256

//============================================================================
void decToBinary(int n, char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }

}

// ============================================================================

class FsFile {
    int file_size;
    int block_in_use;
    int index_block;
    int block_size;// num of chars will save per block.


public:
    FsFile(int _block_size) {
        file_size = 0;
        block_in_use = 0;
        block_size = _block_size;
        index_block = -1;

    }

    int getfile_size(){
        return file_size;
    }

    void setFileSize(int fileSize) {// this int might to be updated.
        file_size += fileSize;
    }

    int getBlockInUse() const {
        return block_in_use;
    }

    void setBlockInUse(int blockInUse) {// this int might to be updated.
        block_in_use += blockInUse;
    }

    int getIndexBlock() const {
        return index_block;
    }

    void setIndexBlock(int indexBlock) {// indexBlock stay the same everytime.
        index_block = indexBlock;
    }

};

// ============================================================================

class FileDescriptor {
    string file_name;
    FsFile* fs_file;
    bool inUse;

public:

    FileDescriptor(string FileName, FsFile* fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }
    FileDescriptor(){//Default Constructor--> in order to remove file...
        file_name = "";
        fs_file = NULL;
        inUse = false;
    }

    string getFileName() {
        return file_name;
    }

    void setFileName(const string &fileName) {
        file_name = fileName;
    }

    FsFile * getFsFile(){
        return fs_file;
    }

    void setFsFile(FsFile *fsFile) {// in case the user want to delete a file...
        file_name = "";
        fs_file = NULL;
        inUse = false;
    }

    bool getIsInUse(){
        return inUse;
    }

    void setInUse(bool inUse) {
        FileDescriptor::inUse = inUse;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

// ============================================================================

class fsDisk {
    int block_size_user=-1;

    FILE * sim_disk_fd;// pointer to our Disk (our file)
    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // (5) MainDir --
    vector <FileDescriptor>  MainDir;
    // Structure that links the file name to its FsFile

    // (6) OpenFileDescriptors --
    vector <FileDescriptor> OpenFileDescriptors;
    //  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.

    // ------------------------------------------------------------------------
public:
    fsDisk() {
        sim_disk_fd = fopen(DISK_SIM_FILE , "w+"); // fsDisk is the Disk for our simulation. it must be w+ cause if not my program will fail!
        assert(sim_disk_fd);

        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd);
            assert(ret_val == 1);
        }

        fflush(sim_disk_fd);
        is_formated = false;
    }

    // ------------------------------------------------------------------------
    ~fsDisk() {// destructor
        delete BitVector;
        BitVector= nullptr;

        for (int i = 0; i < MainDir.size(); i++) {
            delete [](MainDir[i].getFsFile());
        }
        OpenFileDescriptors.clear();

        fclose(sim_disk_fd);

    }


    // ------------------------------------------------------------------------

    void listAll() {

        int i = 0;

        for (auto j = begin(MainDir); j != end(MainDir); ++j) {
            if(j->getFileName()==""){
                i+=2;

            }
            else{
                cout << "index: " << i << ": FileName: " << j->getFileName()  <<  " , isInUse: " << j->getIsInUse() << endl;
                i++;
            }

        }

        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            cout << "(";
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
            cout << ")";
        }
        cout << "'" << endl;
    }



    // ------------------------------------------------------------------------
    // when the user want to format the disk I delete the whole data from, everywhere
    void fsFormat( int blockSize) {

        if(blockSize>DISK_SIZE){
            cout<<"\nYOU CANT ALLOCATE BLOCK SIZE WITH SIZE THAT BIGGER THAN YOUR DISK SIZE!\n";
        }
        block_size_user=blockSize;
        // delete the malloc
        delete []BitVector;
        BitVector=nullptr;

        BitVectorSize = DISK_SIZE / blockSize;
        BitVector = new int[BitVectorSize];
        if(BitVector==nullptr){//malloc failed
            perror("\nfailed with malloc! \n");
            exit(1);
        }

        //first of we know that at the first approach the whole disk is empty,
        //so I want to internalize all of it with zeros
        for(int j=0; j< BitVectorSize; j++){
            BitVector[j]=0;
        }// initialize my bit vector with zeros

        for (int i = 0; i < MainDir.size(); i++) {
            delete [](MainDir[i].getFsFile());
        }// delete fsFile
        MainDir.clear();
        OpenFileDescriptors.clear();
        is_formated=true;// very important for the next functions

        //now I want to intel the disk

        for (int i = 0; i < DISK_SIZE; i++)
        {
            fseek(sim_disk_fd, i, SEEK_SET);
            fwrite("\0", 1, 1, sim_disk_fd);
        }
        printf("FORMAT DISK: number of blocks: %d\n",BitVectorSize);
    }

    // ------------------------------------------------------------------------
    // we got a file name, now we have to create fsFile, and in addition we must update MainDir, OpenFileDescriptors.
    //When the user asks for create a new file it will create with bitUse=1.
    int CreateFile(string fileName) {

        if(!is_formated){// you must format your disk before starts
            cout<<"\n --PLEASE FORMAT YOUR DISK BEFORE-- \n";
            return -1;
        }


        //First, I must know if I have free place in my DISK.
        //For a new file I must have at least 2 free blocks, I will find it in my bitVector...
        int zeros_counter=0;
        int i=0;
        while (i<BitVectorSize){
            if(BitVector[i]==0)
                zeros_counter++;
            if(zeros_counter>=2)
                break;
            i++;
        }
        if(zeros_counter<2){
            cout<<"\n --YOUR DISK IS FULL!-- \n";
            return -1;
        }
        // before I continue I have to know that this file is a new one...
        i=0;
        while( i < MainDir.size()){
            if(MainDir[i].getFileName()==fileName) {// so this file name is already exist...
                cout<< "\n--YOUR FILE IS ALREADY EXISTS!--\n";
                return -1;
            }
            i++;
        }
        int free_block=0;
        while (free_block < BitVectorSize){
            if(BitVector[free_block]==0)// here is a free block
                break;
            free_block++;
        }
        BitVector[free_block]=1;// now this index is occupied.

        //if I got this line, so I know I have enough place in my disk, my file wasn't created before, and it formatted.
        //so, now I am going to locate my blocks.
        FsFile * fs;
        int k=0;
        bool flag=false;
        auto j = begin(MainDir);
        for(; j != end(MainDir); ++j, k++) {
            fs=MainDir[k].getFsFile();
            if(fs==NULL){// I used previous place...
                flag=true;
                break;
            }
        }
        int return_val=k;
        bool found=false;
        fs=new FsFile(block_size_user);
        if(fs==NULL) {
            perror("\nfailed with malloc! \n");
            exit(1);
        }
        if(!flag){
            // now I want to update the MainDir and OpenFileDescriptor
            auto *fd=new FileDescriptor(fileName, fs);// new fd
            if(fd==NULL){//malloc failed.
                perror("\nfailed with malloc! \n");
                delete fs;
                exit(1);
            }
            MainDir.push_back(*fd);
            found=false;
            i=0;
            while( i < OpenFileDescriptors.size()){
                if(OpenFileDescriptors[i].getFileName()=="" && OpenFileDescriptors[i].getFsFile()==nullptr ) { //find place
                    found = true;
                    OpenFileDescriptors.insert(OpenFileDescriptors.begin()+i, MainDir[MainDir.size()-1]);
                    OpenFileDescriptors.erase(OpenFileDescriptors.begin()+i+1);

                    break;
                }
                i++;
            }
            if(!found){
                OpenFileDescriptors.insert(OpenFileDescriptors.end(), MainDir[MainDir.size()-1]);//insert in the end of the OpenFD.
            }

            return_val=i;
            delete fd;
        }
        else{
            auto *fd=new FileDescriptor(fileName, fs);// new fd
            MainDir.insert(MainDir.begin()+k,*fd);//The place in the MainDir is no matter.
            k++;
            MainDir.erase(MainDir.begin()+k+1);
            found=false;
            i=0;
            while( i < OpenFileDescriptors.size()){
                if(OpenFileDescriptors[i].getFileName()=="") { //find place
                    found = true;
                    OpenFileDescriptors.insert(OpenFileDescriptors.begin()+i, MainDir[MainDir.size()-1]);
                    OpenFileDescriptors.erase(OpenFileDescriptors.begin()+i+1);

                    break;
                }
                i++;
            }
            if(!found){
                OpenFileDescriptors.insert(OpenFileDescriptors.end(), MainDir[MainDir.size()-1]);
            }

            delete fd;

        }

        fs->setIndexBlock(free_block);// this index was found earlier, the first free index.

        return return_val;
    }
    // ------------------------------------------------------------------------
    //If the user asks for open a new file, it will be given a new free fd.
    int OpenFile(string fileName) {
        if(!is_formated){
            cout<<"\n --PLEASE FORMAT YOUR DISK BEFORE-- \n";
            return -1;
        }
        //first, I have to check if the file is exist...
        bool found=false;
        int i=0;
        while( i < MainDir.size() && !found){
            if(MainDir[i].getFileName()==fileName) { // so this file name is already exist...
                found = true;
                break;
            }
            i++;
        }
        if(!found) {
            cout << "\n--YOUR FILE IS NOT EXISTS, SO YOU CAN'T OPEN IT!--\n";
            return -1;
        }
        // now I have to know if this file is already open.
        int ind_place=i;

        if(MainDir[i].getIsInUse()){
            cout << "\n--YOUR FILE IS ALREADY OPEN!--\n";
            return -1;
        }
        MainDir[i].setInUse(true);// so let the bit be in using mode .

        found=false;
        i=0;
        while( i < OpenFileDescriptors.size()){
            if(OpenFileDescriptors[i].getFileName()=="" ) {
                found = true;
                OpenFileDescriptors.insert(OpenFileDescriptors.begin()+i, MainDir[ind_place]);
                OpenFileDescriptors.erase(OpenFileDescriptors.begin()+i+1);

                break;
            }
            i++;
        }
        if(!found){
            OpenFileDescriptors.insert(OpenFileDescriptors.end(), MainDir[ind_place]);
        }

        return i;
    }
//    // ------------------------------------------------------------------------
// if the user asks for close a file I have to delete his fd and to change his bit use.
    string CloseFile(int fd) {
        string error0="-1";

        if(!is_formated){
            cout<<"\n --PLEASE FORMAT YOUR DISK BEFORE-- \n";
            return error0;
        }

        if (fd >= OpenFileDescriptors.size() ||OpenFileDescriptors[fd].getFileName()==""){
            printf("\nTHIS FILE DONT EXIST!\n");
            return error0;
        }
        FsFile* fs = OpenFileDescriptors[fd].getFsFile();
        if (!OpenFileDescriptors[fd].getIsInUse()){
            printf("\nTHIS FILE IS ALREADY CLOSE!\n");
            return error0;
        }


        string name=OpenFileDescriptors[fd].getFileName();// The file's name I have to close.
        for(int i=0 ; i<MainDir.size() ;i++){
            if(name== MainDir[i].getFileName()){
                MainDir[i].setInUse(false);
                break;
            }
        }
        OpenFileDescriptors[fd].setInUse(false);
        OpenFileDescriptors[fd].setFsFile(fs);

        return name;
    }
//    // ------------------------------------------------------------------------
//When the user want to write somthing to a file, he has to insured that is file is open, and he must know the right fd in order to write is data.
    int WriteToFile(int fd, char *buf, int len ) {

        if(!is_formated){// must be formated before write int it.
            cout<<"\n --PLEASE FORMAT YOUR DISK BEFORE-- \n";
            return -1;
        }

        if(fd>=OpenFileDescriptors.size()){
            cout<< "\nTHIS FILE IS CLOSE!\n";
            return -1;
        }
        FsFile* fs = OpenFileDescriptors[fd].getFsFile();
        if(OpenFileDescriptors[fd].getFsFile()== nullptr){
            cout<< "\nYOU HAVE SOME PROBLEMS!\n";
            return -1;
        }

        //before I start I have to know if there is enough space in my disk...


        //if we got this line so, I know my file is open and formatted.
        if((len+fs->getfile_size())>(block_size_user*block_size_user)) {// sure our file is Taking up all the space given to him.
            cout<<"\n --YOUR FILE IS FULL, PLEASE CLEAN YOUR DISK..-- \n";
            return -1;
        }
        if(len>(block_size_user*block_size_user)){// check if there is enough space in the file
            cout<<"\n --TOO MANY CHARS PER ONE FILE!..-- \n";
            return -1;
        }

        if(fd>=MainDir.size()){
            cout<< "\nTHIS FILE DONT EXIST!\n";
            return -1;
        }


        //I want an array that will tell me which blocks are free to use...
        int free_blocks_ind[block_size_user];// array that hand free blocks index
        memset(free_blocks_ind, -1,block_size_user);// -1 mean it is not a free place...
        int zeros_counter=0;
        int i=0, j=0;
        while (j<block_size_user){
            if(BitVector[i]==0) {
                free_blocks_ind[j] = i;
                j++;
            }
            i++;
        }
        if(free_blocks_ind[0]==-1){
            cout<< "\n--YOUR DISK IS FULL!--\n";
            return -1;
        }
        //now I want to know if I have any free places at blocks who used for this file.
        // how can I know how many space I have in the last block?
        // file size is the number of chars per this file till now
        //block in use is telling me how many block are in use till now

        char* block_buffer = new char[block_size_user];
        if(block_buffer==NULL){
            perror("\nfailed with malloc! \n");
            exit(1);
        }

        int blocks_to_allocate=len/block_size_user;// for example 10 chars, len=10, bock_size=4, so I need 2 blocks...
        int offset_blocks_to_allocate=len%block_size_user;//offset is another block this case example: 2 full blocks, another one for another 2 chars...
        if(offset_blocks_to_allocate!=0){
            blocks_to_allocate++;
        }
        bool first_time=false;
        if(fs->getBlockInUse()==0)
            first_time=true;


        int offset=fs->getfile_size()%block_size_user;
        if((fs->getBlockInUse()+blocks_to_allocate) <=block_size_user+1 ){
            int unUse_space=0;
            if(fs->getBlockInUse()>0 && offset!=0 ){
                unUse_space=block_size_user-offset;// tell me how many chars I can write in the block.
            }
            // else offset = 0 so unUse_space ==0
            if((offset==0&& blocks_to_allocate>=j )|| (offset>0 && ((len-offset)/block_size_user)>=j)){
                cout<<"\nYOUR DISK DONT HAVE ENOUGH BLOCKS FOR YOUR DATA!\n";
                delete[](block_buffer);
                return -1;
            }

            fseek(sim_disk_fd, fs->getIndexBlock()*block_size_user, SEEK_SET);
            fread(block_buffer, block_size_user, 1, sim_disk_fd);
            int r= block_buffer[fs->getBlockInUse()-1]-48;
            if(fs->getBlockInUse()==0){
                r=block_buffer[fs->getBlockInUse()];
            }
            fseek(sim_disk_fd, (((r+1)*block_size_user)- unUse_space), SEEK_SET);
            int k=0;
            for( ; k<unUse_space && k< len; k++){
                fwrite(&buf[k], sizeof(char), 1, sim_disk_fd);
            }
            // for now, I am sure my last block is full.
            // now I have to Write my others chars, --->

            for(i=0; i<blocks_to_allocate; i++){
                if((len-k)>0 || first_time) {
                    fseek(sim_disk_fd, fs->getIndexBlock() * block_size_user + fs->getBlockInUse(), SEEK_SET);
                    fs->setBlockInUse(1);

                    string tmp = to_string(free_blocks_ind[i]);
                    char block = '0';
                    decToBinary(std::stoi(tmp),block);
                    char const *num_char = &block;
                    fwrite(num_char, 1, 1, sim_disk_fd);
                    BitVector[free_blocks_ind[i]]=1;
                }

                fseek(sim_disk_fd, free_blocks_ind[i]*block_size_user, SEEK_SET);
                for (int a=0;k<=len && a<block_size_user ; k++, a++) {
                    fwrite(&buf[k], sizeof(char), 1, sim_disk_fd);
                }
            }
        }
        fs->setFileSize(len);
        delete[](block_buffer);
        return len;

    }

    int ReadFromFile(int fd, char* buf, int len) {

        if(!is_formated){
            cout<<"\n --PLEASE FORMAT YOUR DISK BEFORE-- \n";
            return -1;
        }

        if (fd >= MainDir.size()){
            cout<< "\nTHIS FILE DONT EXIST!\n";
            return -1;
        }
        if (fd >= OpenFileDescriptors.size() ||OpenFileDescriptors[fd].getFileName()==""){
            cout<< "\nTHIS FILE MIGHT BE CLOSE!\n";
            return -1;
        }

        if (!OpenFileDescriptors[fd].getIsInUse()){
            printf("\nTHIS FILE IS CLOSE!\n");
            return -1;
        }
        FsFile * fs=OpenFileDescriptors[fd].getFsFile();


        if(fs->getfile_size()<len){
            printf("\nYOUR FILE IS TOO SMALL FOR THE LEN YOU TYPED!\n");
            return -1;
        }

        int place=0;
        int j=0;
        int last_place=0;
        char* buffer = new char[block_size_user];
        if(buffer==NULL){
            perror("\nfailed with malloc! \n");
            exit(1);
        }
        fseek(sim_disk_fd, fs->getIndexBlock() * block_size_user , SEEK_SET);
        fread(buffer, block_size_user, 1, sim_disk_fd);
        for(int i=0; buffer[i]!= '\0'; i++){
            place=(int)(buffer[i]-48);
            fseek(sim_disk_fd, place* block_size_user, SEEK_SET);
            for(int k=0; j<len && k<block_size_user ; j++, k++){
                fread(&buf[j], 1, 1, sim_disk_fd);
                last_place++;
            }
            j=last_place;
        }
        buf[j]='\0';
        delete [] buffer;
        return 0;
    }
//--------------------------------------------------------------------------------------------
//In case the user wants to delete the whole data about a file...
    int DelFile(string fileName){
        //I have to delete the file from the MainDir, OpenFD and
        if(!is_formated){
            cout<<"\n --PLEASE FORMAT YOUR DISK BEFORE-- \n";
            return -1;
        }


        bool found=false;
        int ind_found=0;
        int fd=-1;
        //first, if the file is existed it must be on the MainDir vector.
        int i=0;
        while( i < MainDir.size() ){
            if(MainDir[i].getFileName()==fileName){
                found= true;
                ind_found=i;
                break;
            }
            i++;
        }

        FsFile* fs;
        if(!found){// if the file is don't exist I don't have to erase...
            cout<<"\n --THIS FILE IS DOESNT EXIST!-- \n";
            return -1;
        }
        else{
            fs = MainDir[i].getFsFile();
            int ind=fs->getIndexBlock();
        }
        int ind_found0=-1;
        //else, my file is close. So, I want to know if it is open.
        if(MainDir[i].getIsInUse()){// if yes I want to erase the data on it in the OpenFd, because now I know it is open.
            for(i=0; i<OpenFileDescriptors.size(); i++){
                if(OpenFileDescriptors[i].getFileName()==fileName){
                    OpenFileDescriptors[i].setFsFile(fs);
                    ind_found0=i;
                    break;
                }
            }
        }
        char buf[DISK_SIZE];
        int len=fs->getfile_size();
        int place=0;
        int j=0;
        int last_place=0;
        char* buffer = new char[block_size_user];
        if(buffer==NULL){
            perror("\nfailed with malloc! \n");
            exit(1);
        }
        fseek(sim_disk_fd, fs->getIndexBlock() * block_size_user , SEEK_SET);
        fread(buffer, block_size_user, 1, sim_disk_fd);
        for(i=0; buffer[i]!= '\0'; i++){
            place=(int)(buffer[i]-48);
            fseek(sim_disk_fd, place* block_size_user, SEEK_SET);
            for(int k=0; j<len && k<block_size_user ; j++, k++){
                fwrite("\0", 1, 1, sim_disk_fd);
                last_place++;
            }
            j=last_place;
        }


        // now, I want to erase my indexFile.

        fseek(sim_disk_fd, fs->getIndexBlock() * block_size_user , SEEK_SET);
        BitVector[fs->getIndexBlock()]=0;
        for(i=0; buffer[i] != 0 ; i++){
            BitVector[buffer[i]-48]=0;
            fwrite("\0", 1, 1, sim_disk_fd);
        }
        //now after I know that me disk is empty from this file, I can remove it from my MainDir.
        delete[] buffer;
        delete fs;
        MainDir.erase(MainDir.begin()+ ind_found);
        return ind_found0;
    }


} ;

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;
    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );

                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            default:
                break;
        }
    }
}