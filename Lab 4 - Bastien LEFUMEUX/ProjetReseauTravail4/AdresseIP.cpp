#include <stdio.h>

#include <winsock2.h>

#include <ws2tcpip.h>

#include <iostream>

#include <fstream>

#include <string>

#include <clocale>

#include <thread>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

boolean Recherche(string);
void Recherche_thread(string, int, int);

string AppRecherche = "GT-AC5300-9100"; //Nom de l'appareil recherché
string interval_adresseip = "192.168.x.y"; //On veut connaitre les équipements de plage données et on part du principe que l'addresse soit en /16

void main(int argc, char ** argv) {
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int nRet;
    thread threads[4];

    setlocale(LC_ALL, "french"); //  permet d'afficher les caractéres accentués français

    nRet = WSAStartup(wVersionRequested, & wsaData);
    if (wsaData.wVersion != wVersionRequested) {
        cout << "Wrong version";
        return;
    }

    //Lancement de threads pour augmenter la vitesse du scan réseau
    threads[0] = thread(Recherche_thread, interval_adresseip, 0, 63);
    threads[1] = thread(Recherche_thread, interval_adresseip, 64, 127);
    threads[2] = thread(Recherche_thread, interval_adresseip, 128, 191);
    threads[3] = thread(Recherche_thread, interval_adresseip, 192, 255);

    threads[0].join();
    threads[1].join();
    threads[2].join();
    threads[3].join();

    WSACleanup();
}

//Commande lancée par les threads
void Recherche_thread(string adresseip, int min_interval, int max_interval) {
    for (min_interval; min_interval <= max_interval; min_interval++) {
        string tempIp = adresseip;
        for (int k = 0; k < tempIp.length(); k++) {
            if (tempIp[k] == 'x') {
                tempIp.replace(k, 1, to_string(min_interval));
                for (int i = 1; i < 256; i++) {
                    string tempIp2 = tempIp;
                    for (int j = 0; j < tempIp2.length(); j++) {
                        if (tempIp2[j] == 'y') {
                            tempIp2.replace(j, 1, to_string(i));
                        }
                    }
                    if (Recherche(tempIp2)) {
                        return;
                    }
                }
            }
        }
    }
}

boolean Recherche(string adresseip) {
    struct sockaddr_in socketinfo;

    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    string Nom;
    DWORD dwRetval;

    socketinfo.sin_family = AF_INET;

    inet_pton(AF_INET, adresseip.c_str(), & socketinfo.sin_addr);

    dwRetval = getnameinfo((struct sockaddr * ) & socketinfo,
        sizeof(struct sockaddr),
        hostname,
        NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);

    if (dwRetval == 0) // si le serveur a r�pondu
    {
        Nom = hostname; // initialisation de la string Nom (pour faciliter les opérations de comparaison)
        if (adresseip != Nom) // si la getnameinfo n'a pas simplement retourné l'adresse IP mais un véritable nom d'ordinateur
        {
            if (adresseip.compare(AppRecherche) == 0) {
                //On stocke le résultat voulu dans un fichier nommé fichier.txt
                ofstream myfile;
                myfile.open("fichier.txt");
                myfile << "Nom trouvé pour " << adresseip << ": " << Nom << "\n";
                myfile.close();
                return true;
            }
            cout << "Nom trouvé pour " << adresseip << ": " << Nom << "\n";
        } else {
            cout << "Aucun nom trouv� pour l'adresse: " << adresseip << "\n";
        }
    }
    return false;
}