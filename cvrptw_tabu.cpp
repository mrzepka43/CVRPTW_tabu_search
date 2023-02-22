/*
Jędrzej Mikołajczyk Mateusz Rzepka 
Capacitated Vehicle Routing Problem with Time Windows
Tabu search
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>

using namespace std;

// rozmiar sasiadztwa
const int neighbourhood_size = 50;

// czas wykonywania programu
const double work_time = 175.0;

// nazwa pliku z wynikiem
string output_name = "wynik.txt";

// zmienna liczaca numer ciezarowki
int vnr=0;

// Struktura magazynu klienta
struct customer{
    int no;
    int x_coord;
    int y_coord;
    int demand;
    int ready;
    int due;
    int service_time;
    bool visited=false;
};

// struktura ciezarowki
struct vehicle{
    int x_coord;
    int y_coord;
    int capacity;
    double total_distance = 0.0;
    vector<int>clients_id;
};

// utworzenie wskaznika na tablice ciezarowek
vehicle* veh1;

// zmienna okrezlajaca czy podczas przeszukiwania sasiedztwa zmieniono ilosc ciezarowek
bool changed_vnr = false;

// zaczynamy mierzyc czas
auto start = std::chrono::high_resolution_clock::now();

 void clean_file(char* input_file){
    fstream in_file;
    string line;
    string text;
    in_file.open(input_file, ios::in);
    while (getline(in_file, line)) {
		if (!(line.empty() || line.find_first_not_of(' ') == string::npos)) {
            if(line.length()==1){}
            else{
			text += line + "\n";
            }
		}
	}

	in_file.close();

	fstream out_file;
    out_file.open(input_file, ios::out);
	out_file << text;
	out_file.close();
} 

// funkcja czytajaca dane z pliku
vector <customer> read_file(int* cap, char* input_file){
    vector <customer> list_of_clients;
    string bin;
    fstream file;
    file.open(input_file, ios::in);
    if (file.good() == false){
        cout<<"Plik nie zostal otwarty"<<endl;
        exit(0);
    }
    else{
            getline(file,bin);   
            getline(file,bin);
            getline(file,bin);
            file>>bin>>*cap;
            getline(file,bin);
            getline(file,bin);
            getline(file,bin);
            int i=0;
            list_of_clients.push_back(customer());
            while(file>>list_of_clients[i].no>>list_of_clients[i].x_coord>>list_of_clients[i].y_coord>>list_of_clients[i].demand>>list_of_clients[i].ready>>list_of_clients[i].due>>list_of_clients[i].service_time){
                list_of_clients.push_back(customer());
                i++;
            }
            list_of_clients.pop_back();
            file.close();
            return list_of_clients;
    }
}

// funkcja obliczajaca dystans miedzy ciezarowka a klientem
double distance(vehicle a, customer b){
    double x=abs(a.x_coord-b.x_coord);
    double y=abs(a.y_coord-b.y_coord);
    double s=sqrt(x*x+y*y);
    return s;
}

// funkcja obliczajaca dystans miedzy klientami
double distance_between_customers(customer a, customer b){
    double x=abs(a.x_coord-b.x_coord);
    double y=abs(a.y_coord-b.y_coord);
    double s=sqrt(x*x+y*y);
    return s;
}

//funkcja sprawdzajaca, czy wierzcholek podany jako 1 argument jest blizej obecnego miejsca ciezarowki niz 2
bool nearer_client(const customer cus_a, const customer cus_b){
    double x = distance(veh1[vnr], cus_a);
    double y = distance(veh1[vnr], cus_b);
    return x < y;
}

vector <customer> sort_customers(vector <customer> list_of_clients, int i){
    sort(list_of_clients.begin()+i, list_of_clients.end(), nearer_client);
    return list_of_clients;
}

void write_to_file_good_solution(vehicle* trucks, double total_road, char* input_file){
    fstream file;
    file.open(output_name, ios::out);
    if (file.good() == false){
        cout<<"Plik nie zostal otwarty"<<endl;
        exit(0);
    }
    else{
        file<<vnr<<" "<<fixed<<total_road<<endl;
        for (int i=0; i<vnr; i++){
            for(int j=0;j<(int)trucks[i].clients_id.size();j++){
                file<<trucks[i].clients_id[j]<<" ";
            }
            file<<"\n";
    }
    file.close();
}
}

void write_to_file_bad_solution(char* input_file){
    fstream file1;
    file1.open(output_name, ios::out);
    if (file1.good() == false){
        cout<<"Plik nie zostal otwarty"<<endl;
        exit(1);
    }
    else{
        file1<<"-1"<<endl;
    file1.close();
    }
}

vector <customer> greedy_algorithm(vector <customer> list_of_clients, vector<string>* roads, int cap, char* input_file){
    double old_distance_state = 0.0;
    int customers_visited = 0;
    string sroad;
    // szukaj klientow poki nie odwiedzisz wszystkich
    while (customers_visited <= static_cast<int>(list_of_clients.size())){
        int i=1;
        list_of_clients=sort_customers(list_of_clients, i);
        bool anything_happened=false;
        // przeglada liste klientow i sprawdza ktory nie zostal jeszcze odwiedzony wybiera tego najbizszego
        while(i<=static_cast<int>(list_of_clients.size() - 1)){
            if(list_of_clients[i].visited==false){
                veh1[vnr].total_distance+=distance(veh1[vnr],list_of_clients[i]);
                if(veh1[vnr].total_distance<=static_cast<double>(list_of_clients[i].due) && veh1[vnr].capacity-list_of_clients[i].demand>=0){
                    old_distance_state = veh1[vnr].total_distance;
                    if(veh1[vnr].total_distance>=static_cast<double>(list_of_clients[i].ready))
                        veh1[vnr].total_distance+=list_of_clients[i].service_time;
                    else
                        veh1[vnr].total_distance=static_cast<double>(list_of_clients[i].ready+list_of_clients[i].service_time);
                    
                    if (veh1[vnr].total_distance + distance_between_customers(list_of_clients[i],list_of_clients[0]) <= static_cast<double>(list_of_clients[0].due)){
                        veh1[vnr].x_coord=list_of_clients[i].x_coord;
                        veh1[vnr].y_coord=list_of_clients[i].y_coord;
                        list_of_clients[i].visited=true;
                        customers_visited++;
                        veh1[vnr].capacity-=list_of_clients[i].demand;
                        int client_number=list_of_clients[i].no;
                        veh1[vnr].clients_id.push_back(client_number);
                        sroad+=to_string(client_number);
                        sroad+=" ";
                        i++;
                        list_of_clients=sort_customers(list_of_clients, i);
                        anything_happened=true;
                    }
                    else{
                        veh1[vnr].total_distance = old_distance_state;
                        veh1[vnr].total_distance-=distance(veh1[vnr],list_of_clients[i]);
                        i++;
                    }
                }  
                else{
                    veh1[vnr].total_distance-=distance(veh1[vnr],list_of_clients[i]);
                    i++;
                }
            }
            else
                i++;
        }
        if (anything_happened==false){
            cout<<"brak rozwiazania"<<endl;
            write_to_file_bad_solution(input_file);
            exit(1);
        }
        veh1[vnr].total_distance+=distance(veh1[vnr],list_of_clients[0]);
        roads->push_back(sroad);
        sroad="";
        vnr++;
        veh1[vnr].capacity = cap;
        veh1[vnr].x_coord=list_of_clients[0].x_coord;
        veh1[vnr].y_coord=list_of_clients[0].y_coord;

        if(customers_visited == static_cast<int>(list_of_clients.size()) - 1){
            return list_of_clients;
        }
    }
    return list_of_clients;
}

// zlicza calkowita droge rzejechana przez flote ciezarowek
double sum_of_paths(vehicle trucks[]){
    double total_path = 0.0;
    for(int i=0; i<vnr; i++){
        total_path+=trucks[i].total_distance;
    }
    return total_path;
}

// sprawdza czy trasa może zostać wykonana
double check_if_can_add(int client_to_move_index, vehicle *trucks, vector<customer> data, vector<int>::iterator iterator,int truck_nr, int cap){
    // funckja obliczajaca droge, zwraca dlugosc drogi jak moze dodac, -1 jak nie
    vehicle old_truck_state = trucks[truck_nr];
    int client_index = 0;
    trucks[truck_nr].total_distance = 0.0;
    trucks[truck_nr].capacity = cap;
    trucks[truck_nr].x_coord=data[0].x_coord;
    trucks[truck_nr].y_coord=data[0].y_coord;
    trucks[truck_nr].clients_id.insert(iterator, client_to_move_index);

    for (int i = 0; i<(int)trucks[truck_nr].clients_id.size();i++){
        client_index = data[trucks[truck_nr].clients_id[i]].no;
        trucks[truck_nr].total_distance+=distance(trucks[truck_nr],data[client_index]);
        if(trucks[truck_nr].total_distance<=static_cast<double>(data[client_index].due) && trucks[truck_nr].capacity-data[client_index].demand>=0){
            if(trucks[truck_nr].total_distance>=static_cast<double>(data[client_index].ready))
                trucks[truck_nr].total_distance+=data[client_index].service_time;

            else
                trucks[truck_nr].total_distance=static_cast<double>(data[client_index].ready+data[client_index].service_time);

            if (trucks[truck_nr].total_distance + distance_between_customers(data[client_index],data[0]) <= static_cast<double>(data[0].due)){
                trucks[truck_nr].x_coord=data[client_index].x_coord;
                trucks[truck_nr].y_coord=data[client_index].y_coord;
                trucks[truck_nr].capacity-=data[client_index].demand;
            }

            else{
                trucks[truck_nr] = old_truck_state;
                return -1.0;
            }
        }

        else{
            trucks[truck_nr] = old_truck_state;
            return -1.0;
        }
    }

    trucks[truck_nr].total_distance+=distance(trucks[truck_nr],data[0]);
    double new_total_distance = trucks[truck_nr].total_distance;
    trucks[truck_nr] = old_truck_state;
    return new_total_distance;
}

// oblicza calą nową droge cięzarówki bez sprawdzania warunków, zakładając że do każdego klienta można dojechać
double new_distance(vehicle* trucks, vector<customer> data, int truck_nr){
    double new_total_distance = 0.0;
    int client_index = 0;
    trucks[truck_nr].total_distance = 0.0;
    trucks[truck_nr].x_coord=data[0].x_coord;
    trucks[truck_nr].y_coord=data[0].y_coord;

    for (int i = 0; i<(int)trucks[truck_nr].clients_id.size();i++){
        client_index = data[trucks[truck_nr].clients_id[i]].no;
        trucks[truck_nr].total_distance+=distance(trucks[truck_nr],data[client_index]);

        if(trucks[truck_nr].total_distance>=static_cast<double>(data[client_index].ready))
            trucks[truck_nr].total_distance+=data[client_index].service_time;

        else
            trucks[truck_nr].total_distance=static_cast<double>(data[client_index].ready+data[client_index].service_time);
        trucks[truck_nr].x_coord=data[client_index].x_coord;
        trucks[truck_nr].y_coord=data[client_index].y_coord;
    }

    trucks[truck_nr].total_distance+=distance(trucks[truck_nr],data[0]);
    new_total_distance = trucks[truck_nr].total_distance;
    return new_total_distance;
}

// szuka dostępnego rozwiązania w sąsiedztwie
vehicle*  find_solution(vector<customer> customers, vehicle trucks[],int capacity){
    vehicle* trucks_copy = new vehicle[vnr];
    copy(trucks,trucks+vnr,trucks_copy);
    srand((int) time(0));
    int truck_id = rand()%vnr;
    int client_id_from_truck = rand()%(trucks_copy[truck_id].clients_id.size());
    int client_to_move_id = trucks_copy[truck_id].clients_id[client_id_from_truck];
    bool found = false;
    vector<int>::iterator new_order;
    vector<int>::iterator for_deleting;
    for_deleting = trucks_copy[truck_id].clients_id.begin() + client_id_from_truck;
    trucks_copy[truck_id].clients_id.erase(for_deleting);
    trucks_copy[truck_id].total_distance = new_distance(trucks_copy, customers, truck_id);

    if(trucks_copy[truck_id].clients_id.size() == 0 && truck_id == vnr)
    {
        changed_vnr = true;
        vnr--;
    }

    else if(trucks_copy[truck_id].clients_id.size() == 0)
    {
        copy(trucks_copy + truck_id + 1, trucks_copy+vnr, trucks_copy + truck_id);
        changed_vnr = true;
        vnr--;
    }
    
    int i = 0;
    while(found == false && i < vnr){
    int j = 0;
        while(found == false && j<(int)trucks_copy[i].clients_id.size()){
            new_order=trucks_copy[i].clients_id.begin()+j;
            double added = -1.0;
            added = check_if_can_add(client_to_move_id,trucks_copy,customers,new_order,i,capacity);
            if(added != -1.0){
                trucks_copy[i].clients_id.insert(trucks_copy[i].clients_id.begin()+j,client_to_move_id);
                trucks_copy[i].total_distance = added;
                found = true;
            }
            j++;
        }
        i++;
    }
    if(found){
        return trucks_copy;
   }
   if(changed_vnr == true){
        vnr++;
        changed_vnr = false;
   }
    return trucks;
}

// przeszukuje sąsiedztwo
vehicle*  search_neighbourhood(vehicle *base_solution, vector<customer> data, int capacity){
    vehicle *current_solution2;
    vehicle *best_solution2 = base_solution;
    double path_of_best = 0.0;
    double path_of_current = 0.0;
    int i = 0;
    int vnr_of_best_solution = vnr;
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    while(i<neighbourhood_size && elapsed.count() < work_time){
        path_of_best = sum_of_paths(best_solution2);
        current_solution2 = find_solution(data,base_solution,capacity);
        path_of_current = sum_of_paths(current_solution2);
        if(path_of_current <= path_of_best)
        {
            best_solution2 = current_solution2;
            vnr_of_best_solution = vnr;
        }
        if (changed_vnr == true)
        {
            vnr++;
        }
        changed_vnr = false;
        i++;
        finish = std::chrono::high_resolution_clock::now();
        elapsed = finish - start;
    }
    if(vnr_of_best_solution != vnr)
    {
        vnr = vnr_of_best_solution;
        changed_vnr = true;
    }
    return best_solution2;
}

// ogólna struktura algorytmu, zainicjowanie listy tabu
vehicle* taboo_algorithm(vehicle *base_solution, vector<customer> data, int capacity){
    vehicle *current_solution = base_solution;
    vehicle *best_solution = base_solution;
    vector<double> taboo_list; 
    double path_of_best = 0.0;
    double path_of_current = 0.0;
    int i = 0;
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    path_of_best = sum_of_paths(best_solution);
    taboo_list.push_back(path_of_best);
    while(elapsed.count() < work_time){
        path_of_best = sum_of_paths(best_solution);
        current_solution = search_neighbourhood(best_solution,data,capacity);
        path_of_current = sum_of_paths(current_solution);
        if(path_of_current <= path_of_best + (0.05 * path_of_best))
        {
            if(find(taboo_list.begin(), taboo_list.end(), path_of_current) == taboo_list.end()){
                best_solution = current_solution;
                if(taboo_list.size() < 5)
                    taboo_list.push_back(path_of_current);
                else{
                    taboo_list.erase(taboo_list.begin());
                    taboo_list.push_back(path_of_current);
                }
            }
        }
         else if (changed_vnr == true)
        {
            vnr++;
        }
        changed_vnr = false;
        finish = std::chrono::high_resolution_clock::now();
        elapsed = finish - start;
        i++;
    }
    return best_solution;
}

int main(int argc, char* argv[]){
    vector <customer> data1;
    vector <string> line;
    int capacity=0;
    int *wsk_capacity;
    wsk_capacity=&capacity;
    clean_file(argv[1]);
    data1=read_file(wsk_capacity, argv[1]);
    veh1 = new vehicle[data1.size()];
    veh1[vnr].capacity=*wsk_capacity;
    veh1[vnr].x_coord=data1[0].x_coord;
    veh1[vnr].y_coord=data1[0].y_coord;
    data1=greedy_algorithm(data1, &line, *wsk_capacity, argv[1]);
    double trasa=0.0;
    sort(data1.begin(),data1.end(),[]( const customer &left, const customer &right )
                 { return ( left.no < right.no ); } );
    vehicle* sol = taboo_algorithm(veh1,data1,*wsk_capacity);
    trasa = sum_of_paths(sol);
    write_to_file_good_solution(sol,trasa, argv[1]);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";

    return 0;
}