/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts_101304022_101267959.hpp>

std::tuple<std::string, std::string, int> simulate_trace(std::vector<std::string> trace_file, int time, std::vector<std::string> vectors, std::vector<int> delays, std::vector<external_file> external_files, PCB current, std::vector<PCB> wait_queue) {

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output
    std::string system_status = "";  //!< string to accumulate the system status output
    int current_time = time;
    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        auto [activity, duration_intr, program_name] = parse_trace(trace);

        if(activity == "CPU") { //As per Assignment 1
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;
        } else if(activity == "SYSCALL") { //As per Assignment 1
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr;
            current_time = time;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", SYSCALL ISR (ADD STEPS HERE)\n";
            current_time += delays[duration_intr];

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        } else if(activity == "END_IO") {
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            current_time = time;
            execution += intr;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", ENDIO ISR(ADD STEPS HERE)\n";
            current_time += delays[duration_intr];

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        } else if(activity == "FORK") {
            auto [intr, time] = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr;
            current_time = time;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your FORK output here
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr)+ ", cloning the PCB\n" ;
            current_time += duration_intr;

            execution += std::to_string(current_time) + ", 0, scheduler called\n" ;

            execution += std::to_string(current_time) + ", 1, IRET\n" ;
            current_time += 1;

            PCB almost_current(current.PID+1, current.PID, current.program_name, current.size, -1);
            if(allocate_memory(&almost_current)){
                wait_queue.push_back(current);
                current = almost_current;
                system_status += "time: " + std::to_string(current_time) + "; current trace: FORK, " + std::to_string(duration_intr)+ "\n" ;
                system_status += print_PCB(current, wait_queue);
            }
            else{
                system_status += "FORK FAIL";
            }
            ///////////////////////////////////////////////////////////////////////////////////////////

            //The following loop helps you do 2 things:
            // * Collect the trace of the child (and only the child, skip parent)
            // * Get the index of where the parent is supposed to start executing from
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_flag = false;
            int parent_index = 0;

            for(size_t j = i; j < trace_file.size(); j++) {
                auto [_activity, _duration, _pn] = parse_trace(trace_file[j]);
                if(skip && _activity == "IF_CHILD") {
                    skip = false;
                    continue;
                } else if(_activity == "IF_PARENT"){
                    skip = true;
                    parent_index = j;
                    if(exec_flag) {
                        break;
                    }
                } else if(skip && _activity == "ENDIF") {
                    skip = false;
                    continue;
                } else if(!skip && _activity == "EXEC") {
                    skip = true;
                    child_trace.push_back(trace_file[j]);
                    exec_flag = true;
                }

                if(!skip) {
                    child_trace.push_back(trace_file[j]);
                }
            }
            i = parent_index;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the child's trace, run the child (HINT: think recursion)
               //run rprogram using recurision with just one line
                //then the exec will handle it
                int tempTime = current_time;
                auto [temp_execution, temp_system_status, addTime] = simulate_trace(   child_trace, 
                                            tempTime, 
                                            vectors, 
                                            delays,
                                            external_files, 
                                            current, 
                                            wait_queue);
                execution += temp_execution;
                system_status += temp_system_status;
                current_time = addTime;
                free_memory(&current);
                current = wait_queue.at(wait_queue.size()-1);
                wait_queue.erase(wait_queue.end());
            ///////////////////////////////////////////////////////////////////////////////////////////


        } else if(activity == "EXEC") {
            //auto [intr, time] = intr_boilerplate(current_time, 3, 10, vectors);
            //current_time = time;
            //execution += intr;

            ///////////////////////////////////////////////////////////////////////////////////////////
            //Add your EXEC output here
            int size = 0;
                    
            for (const auto& file : external_files) {
                if(program_name == file.program_name){
                    size = file.size;
                }
            }
            PCB almost_current(current.PID, current.PPID, program_name, size, -1);
            free_memory(&current);
            if(allocate_memory(&almost_current)){
                auto [intr, time] = intr_boilerplate(current_time, 3, 10, vectors);
                current_time = time;
                execution += intr;

                execution += std::to_string(current_time) + ", " + std::to_string(duration_intr)+ ", Program is " + std::to_string(size) + "Mb large\n" ;
                current_time += duration_intr;

                execution+= std::to_string(current_time) + ", " + std::to_string(15*size)+ ", loading program into memory\n" ;
                current_time += 15*size;

                execution+= std::to_string(current_time) + ", 3, marking partition as occupied\n" ;
                current_time += 3;

                execution+= std::to_string(current_time) + ", 6, updating PCB\n" ;
                current_time += 6;

                execution+= std::to_string(current_time) + ", 0, scheduler called\n" ;

                execution+= std::to_string(current_time) + ", 1, IRET\n" ;
                current_time += 1;
                
                current = almost_current;
                system_status += "time: " + std::to_string(current_time) + "; current trace: EXEC "+program_name+", " + std::to_string(duration_intr)+ "\n" ;
                system_status += print_PCB(current, wait_queue);
                //std::cout<<"RAN";
            }
            else{
                execution+= std::to_string(current_time) + ", 0, EXEC FAIL\n" ;
                system_status += "EXEC FAIL\n";
                allocate_memory(&current);
            }
            //std::cout << print_PCB(current, wait_queue);

             


            ///////////////////////////////////////////////////////////////////////////////////////////


            std::ifstream exec_trace_file(program_name + ".txt");

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }

            ///////////////////////////////////////////////////////////////////////////////////////////
            //With the exec's trace (i.e. trace of external program), run the exec (HINT: think recursion)
                        
            int tempTime = current_time;
            auto [temp_execution, temp_system_status, addTime] = simulate_trace(   exec_traces, 
                                            tempTime, 
                                            vectors, 
                                            delays,
                                            external_files, 
                                            current, 
                                            wait_queue);
            execution += temp_execution;
            system_status += temp_system_status;
            current_time = addTime;
            
            

            ///////////////////////////////////////////////////////////////////////////////////////////

            break; //Why is this important? (answer in report)

        }
    }

    return {execution, system_status, current_time};
}

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //external_files is a C++ std::vector of the struct 'external_file'. Check the struct in 
    //interrupt.hpp to know more.
    auto [vectors, delays, external_files] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    //Just a sanity check to know what files you have
    print_external_files(external_files);

    //Make initial PCB (notice how partition is not assigned yet)
    PCB current(0, -1, "init", 1, -1);
    //Update memory (partition is assigned here, you must implement this function)
    if(!allocate_memory(&current)) {
        std::cerr << "ERROR! Memory allocation failed!" << std::endl;
    }

    std::vector<PCB> wait_queue;

    /******************ADD YOUR VARIABLES HERE*************************/


    /******************************************************************/

    //Converting the trace file into a vector of strings.
    std::vector<std::string> trace_file;
    std::string trace;
    while(std::getline(input_file, trace)) {
        trace_file.push_back(trace);
    }

    auto [execution, system_status, _] = simulate_trace(   trace_file, 
                                            0, 
                                            vectors, 
                                            delays,
                                            external_files, 
                                            current, 
                                            wait_queue);

    input_file.close();

    write_output(execution, "execution.txt");
    write_output(system_status, "system_status.txt");

    return 0;
}
