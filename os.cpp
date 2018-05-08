/*************************************************************************
Created on : November 27, 2017
Description : Simulates an OS at a basic level
Usage: ./os
Build with: g++ -o os Source.cpp -std=c++11
*************************************************************************/

#include <iostream>
#include <vector>
#include <queue>
#include <string>

using namespace std;

int main() {

	/*At the start, your program asks the user three questions :

	How much RAM memory is there on the simulated computer ? 
		Your program receives the number in bytes(no kilobytes or words).I can enter any number up to 4000000000 (4 billions).
		What is the size of a page / frame.
		How many hard disks the simulated computer has ? Enumeration of hard disks starts with 0
	*/
	//Ask for 3 inputs based on the section given above from blackboard
	//Make all of these unsigned because we shouldn't be taking in negitive numbers
	
	unsigned int sys[3]; //Create an array for our system values ( 0 = Ram, 1 = page size/size of frame, 2 = hard drives/harddisks)

	for (int i = 0; i < 3; i++) {
		string input ; //Create a local temp value to check our input (see if it's less than 0)
		string msg; //Holds our msg to the user
		//Check what iternation we are at and cout a message, since there's 3 this will do.
		if (i == 0)
			msg =  "How big is our RAM in bytes?:";
		else if (i == 1) {
			if (sys[0] == 0) {
				cout << "We cannot have a Ram size of zero!" << endl;
				i = -1;
				continue;
			}
			msg = "How big is a page/frame?:";
		}
		else if (i == 2) {
			if (sys[1] == 0) {
				cout << "A page cannot take no space!" << endl;
				i = 0;
				continue;
			}
			if (sys[1] > sys[0]) {
				cout << "We cannot have a page/frame larger than our RAM size! Please enter valid values! (Restarting program)" << endl;
				i = -1;
				continue;
			}
			msg = "How many hard drives do we have on our system?:";
		}
		else {
			cout << "Something went wrong! This is here so we don't crash... Resetting program.";
			i = 0; //Reset our counter
			break; //Restart our loop
		}
		invalid: //Create a label called invalid for invalid inputs, first time using labels, yay.
		cout << msg;  // Print out our question
		getline(cin, input);
		try {
			//Scan the string to see if we got any non-nummeric valids
			for (int i = 0; i < input.size(); i++) {
				if (!isdigit(input[i])) //Check if the character is not a numberic number
					throw 0;
			}
			 
			//We may not need this based on the previous check... Leave this here since it doens't hurt
			if (stoll(input) < 0) //Check if it's a negitive number
				throw - 1; //Throw an expection, we have a catch all below
		
			else
				sys[i] = stoul(input); //If it isn't a negitive number just convert it into a unsigned int and store it 
		}
		catch (...) {
			cout << "You have entered an invalid value, please try again." << endl << endl;
			goto invalid; //Go to the invalid label and take in input again, alternativily we could have assigned i to i -1 and did a continue but why not try labels?
		}
	}
	
	//We received our inputs now we run an indef. loop for commands.
	//We do not have an exit condition so this program will not stop, assume the termination of the process will be done using ctrl- c in linux


/*

	A priority    ‘A’ input means that a new process has been created.This process has the priority priority.For example, the input A 2 means that a new process has arrived.This process has the priority of 2.

		When a new process arrives, your program should create its PCB and allocate memory for it’s first page.

		Also, when a new process is created your program should send it to the ready - queue or allow it to use the CPU right away.

		When choosing a PID for the new process start from 1 and go up.Do NOT reuse PIDs of the terminated processes.

		t         The process that currently uses the CPU terminates.It leaves the system immediately.Make sure you release the memory used by this process.

		d number file_name       The process that currently uses the CPU requests the hard disk #number.It wants to read or write file file_name.

		D number   The hard disk #number has finished the work for one process.

		m address   The process that currently uses the CPU requests a memory operation for the address address.

		S r     Shows what process is currently using the CPU and what processes are waiting in the ready - queue.

		S i      Shows what processes are currently using the hard disks and what processes are waiting to use them.For each busy hard disk show the process that uses it and show its I / O - queue.Make sure to display the filenames for each process.

		S m    Shows the state of memory.For each used frame display the process number that occupies it and the page number stored in it.

		*/
	

	//Set up actual variables for our CPU,Hardware,Frame table, etc


	//CPU stuff
	struct process{//struct for processes
		int priority; //priority of the process
		string file_name; //String for file name if the process needs to use disk
		unsigned int pid; //The process ID
	};
	unsigned int CPU = 0; //Current running process in the CPU

	struct compareProcess { //Create a struct to hold our compare function
		//Overload our () operator to return our priority checking function
		bool operator()(const process& p1,const  process& p2) const {
			return (p1.priority < p2.priority);
		}
	};
	priority_queue<process,vector<process>, compareProcess> CPU_Q; // Waiting queue for our CPU, use a priority queue
	unsigned int id = 0; //Process Ids start at zero and increase from there.

	//List of our processes
	vector<process> pcbs;
	//Create process zero which is just a clean default process that will be used as a base.
	process zero;
	zero.priority = -1; //Make it -1 so it's always replaceable
	zero.pid = 0; //No process will have zero since we start at 1
	pcbs.push_back(zero);

	//Frame table
	struct page { //Create a struct for pages
		unsigned int pid; //Process ID that this page belongs to
		unsigned int pnum; //Page number
		unsigned int timestamp = 0; //Timestamp for page, default at zero for init.
	};
	unsigned int cTimestamp = 0; //Counter for our current time stamp
	unsigned int frame_tbl_size =  (sys[0] / sys[1]); //Our frame size is our ram divided by how big a page is
	vector<page> frame_table(frame_tbl_size); //Our frame table, which is a vector of pages
	//Make a default page to init. our table with
	page default_page;
	default_page.pid = 0;
	default_page.pnum = 0;
	default_page.timestamp = 0;
	//Now store these in our table
	for (int i = 0; i < frame_tbl_size; i++) {
		frame_table[i] = default_page;
	}

	//Disks
	struct Harddrive {
		unsigned int running = 0; //Process ID that is currently running
		queue<unsigned int> HDD_Q; //Waiting queue for this harddrive
	};
	vector<Harddrive> HDDs(sys[2]);
	

	char cmd; //All commands are one character long, use this to check which command we're processing
	string line; //The input line

	for(;;) { //Acts as a infinite loop
		cout << "Enter a command:"; //Prompt for input/command
		getline(cin, line); //Take input from cin stream and store it into line
		//Parse the line

		int pos = line.find_first_not_of(' '); //Position index of first non-space character
		if (pos == string::npos) { //Check first if we have a empty line
			cout << "No command found on line!(Empty line given)" << endl;
			continue;
		}

		cmd = line[pos];//Assign cmd to the character we found
		if (!isalpha(cmd)) { //Check if the cmd is a alphabet!
			cout << "Commands are alphabet characters! Please try again!" << endl;
			continue;
		}

		//Now check and ensure that we have another argument.
		string temp = line.substr(pos + 1, line.size()); //We handled the cmd, just focus on the rest of the line

		// Lets check if we have t command first since it's the only command with no arguments
		if ((pos+1) >= line.size() || temp.find_first_not_of(' ') == string::npos){ //Check if the character next to our command is a space or out of the scope of our string. This means we have zero arguments and the only command we accept with zero arguments is a t command
			if (cmd == 't') {//Check if we have the t command.
				if (CPU == 0) {
					cout << "We have no process to terminate!" << endl;
					continue;
				}
				//The t command terminates the current running process
				for (int i = 0; i < frame_tbl_size; i++) { //Let's clear the frame table of all pages belonging to this process
					if (frame_table[i].pid == CPU)
						frame_table[i] = default_page; //Replace the page belonging to current running process with the default page
				}
				//We don't have to worry about the PCBS vector since if we remove all traces of it's PID, we won't access it.
				//Lets just treat it like an zombie process.
				//Preempt a process if we can
				//We used a priority Queue so the top process has to be the process with highest priority so we get the top and then pop
				if (!CPU_Q.empty()) { //First we must check if the queue is not empty
					CPU = CPU_Q.top().pid; //Look at the top, get the processID and replace CPU with it
					CPU_Q.pop(); //Pop the process from the queue since it's now in the CPU
				}
				else {
					CPU = 0; //Just place a zero as current running process to indicate that no process is running
				}
				continue;//Move onto the next command
			}
		}
		else if (line[pos + 1] != ' ') { //Make sure that our cmd is exactly one character.
			cout << "Commands are one character! Please try again! " << endl;
			continue;
		}
		
		string arg;
		if (temp.find_first_not_of(' ') != string::npos) { //Make sure we have a character in the line and not just spaces
			temp = temp.substr(temp.find_first_not_of(' '), temp.size()); //Substring once again to kill any spaces in the front
			arg = temp.substr(0, temp.find_first_of(' ')); //Grab the 2nd word in the string
		}
		else {
			cout << cmd << " is either invalid or needs more arguments!" << endl;
			continue;
		}
		//Do some checks on the argument, check if it's empty, a character, a number, or an invalid argument
		if (arg.size() <= 0) {
			cout << "Error, invalid argument:" << arg << endl;
			continue;
		}
		else if ((arg.size() == 1) && (isalpha(arg[0]))) { //Check if Arg is of size 1 and is a letter, which should only be accepted if cmd is S
			if (cmd == 'S')
			{
				//Substring again to see if we have a third argument which we will count as invalid.
				if (temp.find_first_of(' ') != string::npos) { //First check if there is a extra argument...
					temp = temp.substr(temp.find_first_of(' '), temp.size()); //Substring so we can work on it next
					if (temp.find_first_not_of(' ') != string::npos) { //Compare/find if there is a character in our new line
						cout << "Invalid use of the " << cmd << " command! We only want 1 arguments, we found 2 or more!" << endl;
						continue;
					}
				}

				char tempC = arg[0]; //Hold the argument character for comparasion. 
				if (tempC == 'r') {
					//Check if process zero is in the CPU, which means no process is running.
					if (CPU == 0) {
						//NOTHING BUT THE FINEST OUTPUT HERE!
						cout << "\n\t/////////////////////////////////////////////////////" << endl;
						cout << "\t//There is currently no process running in the CPU!//" <<  endl;
						cout << "\t/////////////////////////////////////////////////////" << endl << endl;
					}
					else {
						
						cout << "\n\t/////////////////////////////////////////////////////////////" << endl;
						cout << "\t Process " << CPU << " is currently running with a priority level of:" << pcbs[CPU].priority << endl;
						cout << "\t/////////////////////////////////////////////////////////////" << endl << endl;;
					}

					//Displays the ready queue!
					cout << "\t/////////////////////////////////////////////////////" << endl;
					cout << "\t//                CPU Ready Queue                  //" << endl;
					cout << "\t/////////////////////////////////////////////////////" << endl;
					//Since we're using a priority queue we can't use iterator so we need to make a pop > make copy and display > push back into our queue.
					//First let's make a copy queue and start pushing in pop'd items
					priority_queue<process, vector<process>, compareProcess> temp; //Our temp queue to carry over data
					while(!CPU_Q.empty()) {
						process tempP = CPU_Q.top();
						cout << "\t        -Process " << tempP.pid << " with a priority level of " << tempP.priority << endl; //Display the data we want
						temp.push(tempP); //Push this process into our temp queue
						CPU_Q.pop(); //Pop the process since we have it's data in tempP
					}
					//At this point we should have gone through all the processes in our queue and displayed them. Now we have to add them back to our CPU_Q
					while (!temp.empty()) {
						CPU_Q.push(temp.top());//Simply just push the pop'd item since we don't need to display them.
						temp.pop(); //Pop so we're not stuck in a infinite loop
					}
					//The above was costly and unneeded, only upside is that we may save space if we decided to use vectors instead.
					cout << endl;
					continue;//We're done with this command don't waste time going through the rest, ask for another command
				}
				else if (tempC == 'i') {
					cout << "\n/////////////////////////////////////////////////////" << endl;
					cout << "//                   Hard Drives                   //" << endl;
					cout << "/////////////////////////////////////////////////////" << endl;
					for (int i = 0; i < HDDs.size(); i++) {
						cout << "Hard Drive " << i << ":";
						if (HDDs[i].running == 0) {
							cout << "\tCurrently not working on any files for any processes!" << endl;
						}else
						cout << "\tCurrently working on file \"" << pcbs[HDDs[i].running].file_name << "\" for process " << HDDs[i].running << endl;
						cout << "\t\tQueue:" << endl;
						queue<unsigned int> tempQ; //Create a temp queue since we can't randomly access a queue
						while (!HDDs[i].HDD_Q.empty()) { //Loop till we're done with all items in the queue
							unsigned int tempProcess = HDDs[i].HDD_Q.front();
							cout << "\t\t  Process " << tempProcess << " with file \"" << pcbs[tempProcess].file_name << "\""  << endl;
							tempQ.push(tempProcess); //Push it into our temp queue
							HDDs[i].HDD_Q.pop(); //Pop the queue

						}
						//Now reform the queue we had in our orginial queue.
						while (!tempQ.empty()) {
							HDDs[i].HDD_Q.push(tempQ.front()); //Push the front of the queue of our Temp queue back into our orginial queue
							tempQ.pop(); //Pop the TempQ 
						}
					}
					cout << endl;
					continue;
				}
				else if (tempC == 'm') { //Print Frame table 
					cout << endl;

					cout << "/////////////////////////////////////////////////////" << endl;
					cout << "//                Frame Table                      //" << endl;
					cout << "/////////////////////////////////////////////////////" << endl;
					for (int i = 0; i < frame_tbl_size; i++) { //Loop through our frame table and print 
						if(frame_table[i].pid == 0) //Check if this is a default page, let's just print that it's not in use.
							cout << "Frame " << i << ":" << " \tFrame is NOT in use!" << endl;
						else //Else just print the information we need
							cout << "Frame " << i <<":" << "  \tProcessID:" << frame_table[i].pid << "\tPage Number:" << frame_table[i].pnum << "\t\tTime Stamp:" << frame_table[i].timestamp << endl;
					}
					cout << endl;
					continue;
				}
				else {
					cout << "Invalid argument " << temp << " for command " << cmd << "!" << endl;
					continue;
				}
				
			}
			else {
				cout << "Invalid command/format for:" << cmd << endl;
				continue;
			}
		}
		else {
			//If we reach here we know that the second argument needs to be an number/value
			//First try to convert the value into a number
			unsigned int value;
			try {
				for (int i = 0; i < arg.size(); i++) //Scan the argument to see if it all the characters are numberic values
					if (!isdigit(arg[i])) //Check if the character is not a numberic number
				throw 0; //Throw an expection which will kill this line and ask for a new line
				value = stoul(arg); //Convert the argument into a unsigned int (assume only positive inputs are valid)

			}
			catch (...) {
				cout << "Invalid argument(" << arg << ") found for command:" << cmd << endl;
				continue;
			}

			string filename; //Hold the file name
			//Check for a third argument...
			if (temp.find_first_of(' ') != string::npos) { //First check if there is a extra argument...
				if (cmd != 'd')
				{
					cout << "Unexpected command " << cmd << " with 2 or more arguments." << endl;
					continue;
				}
				//Assume we're dealing with a d command
				temp = temp.substr(temp.find_first_of(' '), temp.size()); //Substring so we can work on it next
				if (temp.find_first_not_of(' ') != string::npos) { //See if there's a character that's not a space!
					temp = temp.substr(temp.find_first_not_of(' '), temp.size()); //If so, substring to work with a smaller string
					filename = temp.substr(0, temp.find_first_of(" \n")); //The file name could be named anything so we just grab this argument
					//Do one last check to see if we have more arguments...
					if (temp.find_first_of(' ') != string::npos) //Check if there's anymore spaces to the line which may tell us there's more to the line...
					{
						temp = temp.substr(temp.find_first_of(' '), temp.size()); //Focus in on the rest of the line that we havn't processed
						if (temp.find_first_not_of(' ') != string::npos) //If we find any other character besides a space, we treat it as invalid since more than we were given was given.
						{
							cout << "Invalid line for command:" << cmd << "! We only want a disk number and filename!" << endl;
							continue;
						}
					}
				}
				else { //This will run if we don't find a filename (or a character with a new substringed line.)
					cout << "We expect a filename for the " << cmd << " command!" << endl;
					continue;
				}

				if (value >= HDDs.size() || value < 0 ) { //Check inputsize
					cout << "HDD number " << value << " does not exist!(Start enumeration at zero)" << endl;
					continue;
				}
				if (CPU == 0) { //Check if we have a processor in the CPU
					cout << "No process is currently running in the CPU! We can't move a non-exisitant process!" << endl;
					continue;
				}

				pcbs[CPU].file_name = filename; //Update the process's file name
				//Move the process into the given device (Hard drive #)
				if (HDDs[value].running == 0) { //Check if the Harddrive's working on anything currently
					HDDs[value].running = CPU; //If not might as well push this process into using the HDD
				}
				else {
					HDDs[value].HDD_Q.push(CPU);//Push it into the queue
				}
				//Preempt a process if we can
				//We used a priority Queue so the top process has to be the process with highest priority so we get the top and then pop
				if (!CPU_Q.empty()) { //First we must check if the queue is not empty
					CPU = CPU_Q.top().pid; //Look at the top, get the processID and replace CPU with it
					CPU_Q.pop(); //Pop the process from the queue since it's now in the CPU
				}
				else {
					CPU = 0; //Set CPU to be running process zero which means it's not running anything
				}
				continue;
			}



			else { //If we do not have more than 3 arguments process/check the conditions below.

				if (cmd == 'A') {
					//Lets create a PCB for our new process.
					process new_process;
					new_process.pid = ++id; //Set the ID of our new process to be the next avaiable ID
					new_process.priority = value; //Set the priority to our argument value.
					//Add our new process to our pcbs list at position = to it's ID for constant access time.
					//Down side is that vector may grow to be very big if we have a ton of processes
					pcbs.push_back(new_process);
					//Now lets see if this process goes to the ready queue or uses the CPU immediately 
					if (new_process.priority > pcbs[CPU].priority) { //Check to see if we should preempt
					//Speical case, if this is a cold start or if we're not running anything we do not want process zero in our ready queue so check for that
						if (!CPU == 0)
							CPU_Q.push(pcbs[CPU]); //Add the process that was in the CPU back to the CPU queue
						CPU = new_process.pid; //Update the CPU to display that our new process is currently running
					}
					else {
						CPU_Q.push(pcbs[new_process.pid]); //Just add our new process into our priority queue
					}
					page new_page; //Make a page for this new process
					new_page.pid = new_process.pid; //Just copy over the ID
					new_page.pnum = 0; //Always zero since this is the first page
					new_page.timestamp = ++cTimestamp; //Copy over the time stamp
					bool placed = false; //See if we placed our page
					unsigned int replaceFrame = cTimestamp; //Index of the frame we should replace, should we need it.
					unsigned int replaceIndex;
					for (int i = 0; i < frame_tbl_size; i++) { //Linearly go through the vector of our frame table and see if we can find a spot.
						if (frame_table[i].timestamp < replaceFrame) { //See if we can update the replace frame index, we only wanna see if the timestamp is less than whatever we've recorded so far
							replaceIndex = i;//Just update the value with the frame index we're currently at
							replaceFrame = frame_table[i].timestamp; //Also update our variable that does the checking
						}
						if (frame_table[i].pid == 0) { //If we find a default page entry we can place our new page at this index
							frame_table[i] = new_page; //Simply update/replace the page at this entry with our new page
							placed = true; //Update our bool to state we successfully found a spot
							break;//Break out of the loop we found a spot to put our new page in
						}
					}
					if (!placed)  //Check if we successfully placed the page, if not then we have just replace the entry based on our replaceFrame index
						frame_table[replaceIndex] = new_page; //Simply replace
					//Note that we can just check for a timestamp value of zero and place our page there instead of checking for Pid
					continue;
				}
				else if (cmd == 'D') {
					if (value >= HDDs.size() || value < 0) {
						cout << "HDD number " << value << " does not exist!(Start enumeration at zero)" << endl;
						continue;
					}
					if (HDDs[value].running == 0) { //Check to see if we're trying to end a process that wasn't there to begin with
						cout << "Nothing was running on HDD number " << value  << "!\n";
						continue;
					}
					else {
						//Process is done so move it back to the CPU ready queue
						process temp = pcbs[HDDs[value].running]; //Create a copy of the currently running process
						CPU_Q.push(temp); //Push that process back onto the CPU Q
						//Now check if there's any other processes on the queue that are waiting to use this device
						if (HDDs[value].HDD_Q.empty()) {
							HDDs[value].running = 0; //If the queue is empty then just set the current running processor to zero which is the default stating no process is running
						}
						else {
							unsigned int goNext = HDDs[value].HDD_Q.front(); //Get the value of the front of the queue which we are going to pop
							HDDs[value].running = goNext;  //Get that process activitly working on the Harddrive
							HDDs[value].HDD_Q.pop(); //Pop the front of the stack since it's not running
						}
					}
					if (CPU == 0) { //If there isn't a process already in the CPU, move the one we just pushed into the Queue into the CPU
						process preempt = CPU_Q.top(); //Clone the process at the top of the prioirty Queue
						CPU = preempt.pid; //Assign CPU the process ID
						CPU_Q.pop(); //Pop the top of the queue which we just moved into the CPU
					}
					else {
						process currentlyrunning = pcbs[CPU]; //Grab the PCB id of currently running process
						CPU_Q.push(currentlyrunning); //Push that back into the CPU Q
						CPU = CPU_Q.top().pid; //Take the highest priority from the CPU Q and put it into the CPU
						CPU_Q.pop(); //Pop the process we just added to the CPU since it's in the CPU instead of the ready Q
		

					}
					continue;
				}
				else if (cmd == 'm') {
					if (CPU == 0) { //Do a CPU process check before doing anything
						cout << "Cannot load memory for no process!" << endl;
						continue;
					}
					page new_page;
					new_page.pid = CPU;
					new_page.pnum = value / sys[1];
					new_page.timestamp = ++cTimestamp;
					unsigned int replaceFrame = cTimestamp; //Index of the frame we should replace, should we need it.
					unsigned int replaceIndex;
					bool replace = true;
					for (int i = 0; i < frame_tbl_size; i++) { //Linearly go through the vector of our frame table and see if we can find a spot.
						if (frame_table[i].timestamp < replaceFrame) { //See if we can update the replace frame index, we only wanna see if the timestamp is less than whatever we've recorded so far
							replaceIndex = i;//Just update the value with the frame index we're currently at
							replaceFrame = frame_table[i].timestamp; //Also update our variable that does the checking
						}
						if (frame_table[i].pid == new_page.pid && frame_table[i].pnum == new_page.pnum) {
							frame_table[i].timestamp = new_page.timestamp; //Update the time stamp
							replace = false; //Note that we don't replace we just update timestamp
							break; //Break since we found a match and won't need to scan anymore...
						}
					}
					if(replace) //Check if we need to replace
						frame_table[replaceIndex] = new_page; //If so do so
					continue;
				}
				else {
					cout <<"Given line is invalid! Please try again!" << endl;
					continue;
				}
			}
		}
	}
}