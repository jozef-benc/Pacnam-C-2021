#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>

#define ghost_count 4
#define offset_x 10
#define offset_y 10


using namespace std;
HANDLE konzola = GetStdHandle(STD_OUTPUT_HANDLE);


void gotoxy(short x, short y)
{
    SetConsoleCursorPosition(konzola, {x, y});
}


class Bludisko
{
    private:
        int sirka;
        int vyska;
        int maxskore;
        vector<vector<char>> mapa;

    public:
        Bludisko(int m, int n)
        {
            sirka = m;
            vyska = n;

            mapa.resize(vyska);
            for (int i = 0; i < vyska; i++)
                mapa[i].resize(sirka);

            for (int i = 0; i < vyska; i++)
            {
                for (int j = 0; j < sirka; j++)
                    if (i == 0 || j == 0 || i == vyska-1 || j == sirka-1) mapa[i][j] = 1;
                    else mapa[i][j] = 0;
            }
        }
        ~Bludisko (){}

        Bludisko(string nazovSuboru)
        {
            maxskore = 0;
            nacitanieZoSuboru(nazovSuboru);
        }

        void nacitanieZoSuboru(string nazovSuboru)
        {
            ifstream subor(nazovSuboru);
            if (!subor) return;

            subor >> vyska >> sirka;
            subor.get();

            mapa.resize(vyska);
            for (int i = 0; i < vyska; i++)
                mapa[i].resize(sirka);

            char pismeno;
            for (int i = 0; i < vyska; i++)
            {
                for (int j = 0; j < sirka; j++)
                {
                    subor.get(pismeno);
                    switch (pismeno)
                    {
                        case 'X': mapa[i][j] = 1; break;
                        case '.': mapa[i][j] = 2; maxskore++; break;
                        default: mapa[i][j] = 0;
                    }
                }
                subor.get();
            }
            subor.close();
        }

        bool mozePrejst(int x, int y)
        {
            if (x < 0) x = sirka-1; else
            if (x >= sirka) x = 0;
            if (y < 0) y = vyska-1; else
            if (y >= vyska) y = 0;

            return (mapa[y][x] != 1);
        }

        bool jeJedlo(int x, int y)
        {
            if (mapa[y][x] == 2)
            {
                mapa[y][x] = 0;
                return true;
            }
            return false;
        }

        int dajSirku() {return sirka;}
        int dajVysku() {return vyska;}
        int dajMaxSk() {return maxskore;}
        char dajPolicko(int x, int y) {return mapa[y][x];}

        void vykresli()
        {
            for (int i = 0; i < vyska; i++)
            {
                gotoxy(offset_x, offset_y + i);
                for (int j = 0; j < sirka; j++)
                {
                    SetConsoleTextAttribute(konzola, 0x09);
                    if (mapa[i][j] == 1) cout << "██";
                    else if (mapa[i][j] == 0) cout << "  ";
                    else
                    {
                        SetConsoleTextAttribute(konzola, 0x0E);
                        cout << " •";
                    }
                }
            }
        }

        void vykresli_policko(int x, int y)
        {
            gotoxy(offset_x + x*2, offset_y + y);
            SetConsoleTextAttribute(konzola, 0x09);

            if (mapa[y][x] == 1) cout << "██";
            else if (mapa[y][x] == 0) cout << "  ";
            else
            {
                SetConsoleTextAttribute(konzola, 0x0E);
                cout << " •";
            }
        }
};


class Pohybujuci_objekt
{
     private:
        int x;
        int y;
        int farba;
        char symbol;
        Bludisko *bludisko;

    public:
        Pohybujuci_objekt(){}
        Pohybujuci_objekt(int x, int y, int farba, char symbol, Bludisko *bludisko)
        {
            this->x = x;
            this->y = y;
            this->farba = farba;
            this->symbol = symbol;
            this->bludisko = bludisko;
        }
        ~Pohybujuci_objekt(){}

        int daj_x() {return x;}
        int daj_y() {return y;}


        bool skontroluj_poziciu(int vx, int vy)
        {
            if (x == vx && y == vy) return true;
            return false;
        }

        bool pohni_x(int vx)
        {
            if (bludisko)
            {
                if (bludisko->mozePrejst((x-offset_x)/2+vx, y-offset_y))
                {
                    if ((x-offset_x)/2+vx < 0) x = offset_x+(bludisko->dajSirku()-1)*2;
                    else
                    if ((x-offset_x)/2+vx >= bludisko->dajSirku()) x = offset_x;
                    else
                    x += vx*2;
                    return true;
                }
            }
            else
            {
                if (x+vx*2 >= 0)
                {
                    x += vx*2;
                    return true;
                }
            }
            return false;
        }

        bool pohni_y(int vy)
        {
            if (bludisko)
            {
                if (bludisko->mozePrejst((x-offset_x)/2, y-offset_y+vy))
                {
                    if (y-offset_y+vy < 0) y = offset_y+bludisko->dajVysku()-1;
                    else
                    if (y-offset_y+vy >= bludisko->dajVysku()) y = offset_y;
                    else
                    y += vy;
                    return true;
                }
            }
            else
            {
                if (y+vy >= 0)
                {
                    y += vy;
                    return true;
                }
            }
            return false;
        }

        void vykresli()
        {
            gotoxy(x, y);
            SetConsoleTextAttribute(konzola, farba);
            cout << symbol << symbol;
        }

        void vymaz()
        {
            gotoxy(x, y);
            SetConsoleTextAttribute(konzola, 0x00);
            cout << "  ";
        }

        void prepis()
        {
            bludisko->vykresli_policko((x-offset_x)/2, y-offset_y);
        }

        bool zjedz()
        {
            if (bludisko->jeJedlo((x-offset_x)/2, y-offset_y)) return true;
            return false;
        }
};


class Pacman: public Pohybujuci_objekt
{
    private:
        int skore;
        int zivoty;

    public:
        Pacman() : Pohybujuci_objekt()
        {
            skore = 0;
            zivoty = 3;
        }
        Pacman(int x, int y, char c, Bludisko *bludisko) : Pohybujuci_objekt(x, y, 0x6F, c, bludisko)
        {
            skore = 0;
            zivoty = 3;
        }
        ~Pacman(){}

        int daj_skore() {return skore;}
        int daj_zivoty() {return zivoty;}
        void zober_zivot() {zivoty--;}


        void pohyb()
        {
            if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(0x44))
            {
                vymaz();
                pohni_x(1);
                skore += zjedz();
                vykresli();
            }
            if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(0x41))
            {
                vymaz();
                pohni_x(-1);
                skore += zjedz();
                vykresli();
            }
            if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57))
            {
                vymaz();
                pohni_y(-1);
                skore += zjedz();
                vykresli();
            }
            if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x53))
            {
                vymaz();
                pohni_y(1);
                skore += zjedz();
                vykresli();
            }
        }
};


class Strasidlo: public Pohybujuci_objekt
{
    private:
        int smer;

    public:
        Strasidlo () : Pohybujuci_objekt()
        {
            smer = 0;
        }
        Strasidlo(int x, int y, char c, Bludisko *bludisko) : Pohybujuci_objekt(x, y, 0x4F, c, bludisko)
        {
            smer = 0;
        }
        ~Strasidlo(){}

        void pohyb()
        {
                prepis();

                bool cyklus = true;
                int random = rand() % 4;
                if (abs(random-smer) == 2) random = (random+1)%4;

                while (cyklus)
                {
                    random = (random+1)%4;
                    if (abs(random-smer) == 2) random = (random+1)%4;

                    switch (random)
                    {
                        case 0: if (pohni_x(1)) cyklus = false; break;
                        case 1: if (pohni_y(1)) cyklus = false; break;
                        case 2: if (pohni_x(-1)) cyklus = false; break;
                        case 3: if (pohni_y(-1)) cyklus = false; break;
                    }
                }
                smer = random;

                vykresli();
        }
};



void titulny_text(int x, int y)
{
    SetConsoleTextAttribute(konzola, 0x0F);
    gotoxy(x, y  ); cout << "██████  ██████  ██████  ██    ██  ██████  ██      ██" << endl;
    gotoxy(x, y+1); cout << "██  ██  ██  ██  ██      ████  ██  ██  ██  ████  ████" << endl;
    gotoxy(x, y+2); cout << "██████  ██████  ██      ████████  ██████  ██  ██  ██" << endl;
    gotoxy(x, y+3); cout << "██      ██  ██  ██      ██  ████  ██  ██  ██      ██" << endl;
    gotoxy(x, y+4); cout << "██      ██  ██  ██████  ██    ██  ██  ██  ██      ██" << endl;

    x += 16;
    SetConsoleTextAttribute(konzola, 0x06);
    gotoxy(x, y  ); cout << "██████" << endl;
    gotoxy(x, y+1); cout << "██" << endl;
    gotoxy(x, y+2); cout << "██" << endl;
    gotoxy(x, y+3); cout << "██" << endl;
    gotoxy(x, y+4); cout << "██████" << endl;
}

void vypis_skore(int x, int y, int score)
{
    gotoxy(x, y);
    SetConsoleTextAttribute(konzola, 0x0F);
    printf("%05d", score);
}

void vypis_zivoty(int x, int y, int zivoty, int rest)
{
    gotoxy(x, y);
    SetConsoleTextAttribute(konzola, 0x0C);
    while (zivoty--) printf("♥ ");
    SetConsoleTextAttribute(konzola, 0x09);
    while (rest--) printf("♥ ");
}



int main()
{
    SetConsoleOutputCP(CP_UTF8);
    system("MODE CON COLS=76 LINES=45");
    cout << "© 2021 Jozef Benc" << endl;

    CONSOLE_CURSOR_INFO lpCursor = {20, false};
	SetConsoleCursorInfo(konzola, &lpCursor);

	titulny_text(12, 3);

    Bludisko *bludisko = new Bludisko("mapa.txt");
    bludisko->vykresli();
    int width = bludisko->dajSirku(), height = bludisko->dajVysku();

    Pacman *hrac = new Pacman(offset_x, offset_y+height/2-1, '^', bludisko);
    hrac->vykresli();

    Strasidlo *duch[ghost_count];
    for (int i = 0; i < ghost_count; i++)
    {
        duch[i] = new Strasidlo(offset_x+width-2, offset_y+height/2-1, 'x', bludisko);
        duch[i]->vykresli();
    }

    gotoxy(offset_x, offset_y-1);
    SetConsoleTextAttribute(konzola, 0x0F);
    printf("LEVEL: 1              SCORE: %05d                 ", hrac->daj_skore());


    vypis_zivoty(offset_x+width*2-5, offset_y-1, hrac->daj_zivoty(), 3-hrac->daj_zivoty());

    srand(time(0));

    while(1)
    {
        for (int i = 0; i < ghost_count; i++)
            duch[i]->pohyb();

        if (GetAsyncKeyState(0x45)) break;

        for (int i = 0; i < ghost_count; i++)
            if (hrac->skontroluj_poziciu(duch[i]->daj_x(), duch[i]->daj_y()))
            {
                hrac->zober_zivot();
                vypis_zivoty(offset_x+width*2-5, offset_y-1, hrac->daj_zivoty(), 3-hrac->daj_zivoty());
                Sleep(500);
            }
        if (hrac->daj_zivoty() == 0)
        {
            SetConsoleTextAttribute(konzola, 0x4F);
            gotoxy(offset_x+width-13, offset_y-1);
            printf(" BOHUŽIAĽ, PREHRAL SI! :( ");
            Sleep(2000);
            break;
        }

        hrac->pohyb();

        vypis_skore(offset_x+width+1, offset_y-1, hrac->daj_skore());
        if (hrac->daj_skore() == bludisko->dajMaxSk())
        {
            SetConsoleTextAttribute(konzola, 0x2F);
            gotoxy(offset_x+width-13, offset_y-1);
            printf(" GRATULUJEM, VYHRAL SI! :) ");
            Sleep(5000);
            break;
        }

        Sleep(80);
    }

    delete hrac;
    for (int i = 0; i < ghost_count; i++)
        delete duch[i];
    delete bludisko;

    return 0;
}
