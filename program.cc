#include"Player.hh"
#include<cmath>
#include<string>
#include<iterator>
#include<map>
#include<algorithm>

#define PLAYER_NAME Vasquito

struct PLAYER_NAME : public Player {
  static Player* factory () {
    return new PLAYER_NAME;
  }
  vector<Dir> dirs = {Up,Down,Left,Right};
  typedef vector<vector<Dir>> mapa_dir;
  typedef vector<vector<int>> mapa_vis;
  bool casilla_vacia(Pos const &posicion)
  {
    return(cell(posicion).type == Street and cell(posicion).bonus == NoBonus and cell(posicion).weapon == NoWeapon and cell(posicion).b_owner == -1 and cell(posicion).id == -1);
  }
  bool es_interseccion(Pos const &posicion)
  {
    int cont_1 = 0;
    int cont_2 = 0;
    Pos p_nueva;
    for(int i = -1; i < 2; i+=2)
    {
      for(int j = -1; j < 2; j+=2)
      {
        p_nueva.i = posicion.i + i;
        p_nueva.j = posicion.j + j;
        if(!pos_ok(p_nueva) or cell(p_nueva).type == Building) ++cont_1;
      }
    }

    for(int i = -1; i < 2; i+= 2)
    {
      p_nueva.i = posicion.i + i;
      p_nueva.j = posicion.j + 0;
      if(pos_ok(p_nueva) and cell(p_nueva).type == Street) ++cont_2;
    }

    for(int i = -1; i < 2; i+= 2)
    {
      p_nueva.i = posicion.i + 0;
      p_nueva.j = posicion.j + i;
      if(pos_ok(p_nueva) and cell(p_nueva).type == Street) ++cont_2;
    }

    return (cont_1 >= 3 and cont_2 >= 3);
  }
  bool requisitos_totales(Pos const &posicion)
  {
    return(cell(posicion).type == Street and cell(posicion).id == -1 and (cell(posicion).b_owner == me() or cell(posicion).b_owner == -1));
  }
  bool requisitos_barricadas(Pos const &posicion)
  {
    return(cell(posicion).type == Street and (cell(posicion).b_owner == me() or cell(posicion).b_owner == -1));
  }
  bool requisitos_lado(Pos const &posicion)
  {
    //quito barricada
    return(cell(posicion).type == Street and cell(posicion).id == -1);
  }
  bool es_aliado(int id_aliado)
  {
    return (id_aliado != -1 and citizen(id_aliado).player == me());
  }
  bool es_enemigo(int id_aliado)
  {
    return (id_aliado != -1 and citizen(id_aliado).player != me());
  }
  bool is_bonus(Pos const &p_bonus)
  {
    return(cell(p_bonus).bonus != NoBonus);
  }
  bool is_weapon(Pos const &p_weapon)
  {
    return(cell(p_weapon).weapon != NoWeapon);
  }
  int rad(Pos pa, int b, int c)
  {
    return (abs(pa.i-b) + abs(pa.j-c));
  }
  bool busqueda_centro(Pos const &pos_inicial, Dir &d_f)
  {
    queue<Pos> c_p; //Cola Pos
    mapa_dir m_d(board_rows(), vector<Dir>(board_cols())); //Mapa direcciones
    mapa_vis m_v(board_rows(), vector<int>(board_cols(), 0)); //Mapa visitados
    m_d[pos_inicial.i][pos_inicial.j] = Down; //En las 4 primeras dir asigno la direccion d
    m_v[pos_inicial.i][pos_inicial.j] = 1;
    c_p.push(pos_inicial);
    bool found = true;
    while(!c_p.empty() or !found) //Mientras la cola no este vacia o NO encontrado algo interesante
    {
      Pos p_a = c_p.front();
      c_p.pop();

      for(Dir d : dirs)
      {
        Pos p_b = p_a + d;
        if(!pos_ok(p_b)) continue;
        if(!requisitos_barricadas(p_b))continue;
        if(m_v[p_b.i][p_b.j] != 0) continue;
        if (rad(p_b, board_cols(), board_rows()) < 2)
        {
          d_f = m_d[p_a.i][p_a.j];
          return true;
        }
        c_p.push(p_b);
        m_v[p_b.i][p_b.j] = m_v[p_a.i][p_a.j] + 1;
        if(m_v[p_b.i][p_b.j] == 2)
        {
            m_d[p_b.i][p_b.j] = d;
        }
        else
        {
            m_d[p_b.i][p_b.j] = m_d[p_a.i][p_a.j]; //En las otras asigno las anteriores
        }
      }
    }
    return false;
  }
  bool busqueda_dia_b(Pos const &pos_inicial, Dir &d_f, bool &intr_encontrada)
  {
    int id = cell(pos_inicial).id;
    queue<Pos> c_p;
    mapa_dir m_d(board_rows(), vector<Dir>(board_cols())); //Mapa direcciones
    mapa_vis m_v(board_rows(), vector<int>(board_cols(), 0)); //Mapa visitados
    mapa_vis m_dist(board_rows(), vector<int>(board_cols(), 0)); //Mapa distancias
    m_d[pos_inicial.i][pos_inicial.j] = Down;
    m_v[pos_inicial.i][pos_inicial.j] = 1;
    m_dist[pos_inicial.i][pos_inicial.j] = 1;

    map <Dir,int> mapa_short;
    Dir mejor_d;
    int mejor_p = 0;
    //1) Miramos alrededor cercano
    {
      random_shuffle(dirs.begin(),dirs.end());
      for(Dir d : dirs)
      {
        Pos p_b = pos_inicial + d;
        if(!pos_ok(p_b)) continue;
        if(!requisitos_barricadas(p_b)) continue;
        if(es_interseccion(p_b))
        {
          int num_barr = barricades(me()).size();
          if(num_barr < 3 and casilla_vacia(p_b))
          {
            build(id, d);
            intr_encontrada = true;
            return false;
          }
        }
        else if(is_weapon(p_b))
        {
          if(cell(p_b).weapon == Gun)
          {
            mapa_short[d] = 5;
          }
          else if(cell(p_b).weapon == Bazooka)
          {
            mapa_short[d] = 7;
          }
        }
        else if (cell(p_b).bonus == Money)
        {
          mapa_short[d] = 4;
        }
        else if (cell(p_b).bonus == Food)
        {
          mapa_short[d] = 10 - (citizen(id).life/20.0);
        }
        if(requisitos_totales(p_b))
        {
            c_p.push(p_b);
            m_d[p_b.i][p_b.j] = d; //En las 4 primeras dir asigno la direccion d
            m_dist[p_b.i][p_b.j] = 1;
        }
      }
    }
    {
      map <Dir,int> :: iterator it = mapa_short.begin();
      while(it != mapa_short.end())
      {
        if(it->second > mejor_p)
        {
          mejor_d = it->first;
          mejor_p = it-> second;
        }
        ++it;
      }
      if(mejor_p > 0)
      {
        d_f = mejor_d;
        return true;
      }
    }
    //2) Miramos el mapa entero
    map <Dir,float> mapa_long;
    vector<pair<Dir,float>> prueba;
    bool found = false;
    {
      while(!c_p.empty() and !found)
      {
        Pos p_a = c_p.front();
        c_p.pop();
        random_shuffle(dirs.begin(),dirs.end());
        for(Dir d : dirs)
        {
          Pos p_b = p_a + d;
          if(!pos_ok(p_b)) continue;
          if(!requisitos_barricadas(p_b))continue;
          if(m_v[p_b.i][p_b.j] != 0) continue;
          if(is_weapon(p_b))
          {
            if(cell(p_b).weapon == Bazooka)
            {
              mapa_long[m_d[p_a.i][p_a.j]] += 50.0 / m_dist[p_a.i][p_a.j];
            }
            else if(cell(p_b).weapon == Gun)
            {
              mapa_long[m_d[p_a.i][p_a.j]] += 25.0 / m_dist[p_a.i][p_a.j];
            }
          }
          else if(cell(p_b).bonus == Money)
          {
            mapa_long[m_d[p_a.i][p_a.j]] += 30.0 / m_dist[p_a.i][p_a.j];
          }
          else if (cell(p_b).bonus == Food)
          {
            mapa_long[m_d[p_a.i][p_a.j]] += 100/ (citizen(id).life + m_dist[p_a.i][p_a.j]);
          }
          c_p.push(p_b);
          m_v[p_b.i][p_b.j] = 1;
          m_dist[p_b.i][p_b.j] = m_dist[p_a.i][p_a.j] + 1;
          m_d[p_b.i][p_b.j] = m_d[p_a.i][p_a.j]; //En las otras asigno las anteriores
        }
      }
      {
        mejor_p = 0;
        map<Dir,float> :: iterator it2;
        it2 = mapa_long.begin();
        while(it2 != mapa_long.end())
        {
          if(it2->second > mejor_p)
          {
            mejor_d = it2->first;
            mejor_p = it2-> second;
          }
          ++it2;
        }
        if(mejor_p > 0)
        {
          d_f = mejor_d;
          return true;
        }
      }
      return false;
    }
  }
  bool busqueda_dia_w(Pos const &pos_inicial, Dir &d_f)
  {

    int id = cell(pos_inicial).id;
    queue<Pos> c_p;
    mapa_dir m_d(board_rows(), vector<Dir>(board_cols())); //Mapa direcciones
    mapa_vis m_v(board_rows(), vector<int>(board_cols(), 0)); //Mapa visitados
    mapa_vis m_dist(board_rows(), vector<int>(board_cols(), 0)); //Mapa visitados

    m_d[pos_inicial.i][pos_inicial.j] = Up; //En las 4 primeras dir asigno la direccion d
    m_v[pos_inicial.i][pos_inicial.j] = 1;
    m_dist[pos_inicial.i][pos_inicial.j] = 1;

    map <Dir,int> mapa_short;
    Dir mejor_d;
    int mejor_p = 0;

    //1) Miramos alrededor cercano
    {
      random_shuffle(dirs.begin(),dirs.end());
      for(Dir d : dirs)
      {
        Pos p_b = pos_inicial + d;
        if(!pos_ok(p_b)) continue;
        if(!requisitos_barricadas(p_b)) continue;
        if (citizen(id).type == Warrior){
          if (citizen(id).weapon == Bazooka){
            if (is_weapon(p_b)){
              mapa_short[d] = 5;
            }
            else if (cell(p_b).bonus == Money){
              mapa_short[d] = 3;
            }
            else if (cell(p_b).bonus == Food){
              mapa_short[d] = 10 - (citizen(id).life/20.0);
            }
          }
          else if (citizen(id).weapon == Gun){
            if (is_weapon(p_b)){
              if (cell(p_b).weapon == Bazooka){
                  mapa_short[d] = 10;
                  break;
              }
              mapa_short[d] = 5;
            }
            else if (cell(p_b).bonus == Money){
              mapa_short[d] = 3;
            }
            else if (cell(p_b).bonus == Food){
              mapa_short[d] = 10 - (citizen(id).life/15.0);
            }
          }
          else if (citizen(id).weapon == Hammer){
            if (is_weapon(p_b)){
                mapa_short[d] = 10;
                break;
            }
            else if (cell(p_b).bonus == Money){
              mapa_short[d] = 3;
            }
            else if (cell(p_b).bonus == Food){
              mapa_short[d] = 10 - (citizen(id).life/20.0);
            }
          }
        }
        if(requisitos_totales(p_b))
        {
            c_p.push(p_b);
            m_d[p_b.i][p_b.j] = d; //En las 4 primeras dir asigno la direccion d
            m_dist[p_b.i][p_b.j] = 1;
        }
      }
    }
    {
      map <Dir,int> :: iterator it = mapa_short.begin();
      while(it != mapa_short.end())
      {
        if(it->second > mejor_p)
        {
          mejor_d = it->first;
          mejor_p = it-> second;
        }
        ++it;
      }
      if(mejor_p > 0)
      {
        d_f = mejor_d;
        return true;
      }
    }
//------------------------------------------------------------------------------------------_//
    //2) Miramos el mapa entero
    map <Dir,float> mapa_long;
    vector<pair<Dir,float>> prueba;
    bool found = false;
    {
      while(!c_p.empty() and !found)
      {
        random_shuffle(dirs.begin(),dirs.end());
        Pos p_a = c_p.front();
        c_p.pop();
        for(Dir d : dirs)
        {
          Pos p_b = p_a + d;
          if(!pos_ok(p_b)) continue;
          if(!requisitos_barricadas(p_b))continue;
          if(m_v[p_b.i][p_b.j] != 0) continue;
          if(cell(p_b).type != Street) continue;
          {
            if(citizen(id).type == Warrior)
            {
              if(citizen(id).weapon == Bazooka)
              {
                if(is_weapon(p_b))
                {
                  mapa_long[m_d[p_a.i][p_a.j]] += 200.0 / m_dist[p_a.i][p_a.j];
                }
                else if(cell(p_b).bonus == Money)
                {
                  mapa_long[m_d[p_a.i][p_a.j]] += 25.0 / m_dist[p_a.i][p_a.j];
                }
                else if (cell(p_b).bonus == Food)
                {
                  mapa_long[m_d[p_a.i][p_a.j]] += 100/ (citizen(id).life + m_dist[p_a.i][p_a.j]);
                }
              }
              else if (citizen(id).weapon == Gun)
              {
                if (is_weapon(p_b))
                {
                  if (cell(p_b).weapon == Bazooka)
                  {
                      mapa_long[m_d[p_a.i][p_a.j]] += 100000.0;
                      found = true;
                      break;
                  }
                  mapa_long[m_d[p_a.i][p_a.j]] += 5.0;
                  prueba.push_back({d,5.0});
                }
                else if (cell(p_b).bonus == Money){
                  mapa_long[m_d[p_a.i][p_a.j]] += 50.0 / m_dist[p_a.i][p_a.j];
                }
                else if (cell(p_b).bonus == Food){
                  mapa_long[m_d[p_a.i][p_a.j]] += 100.0/ (citizen(id).life + m_dist[p_a.i][p_a.j]);
                }
              }
              else if (citizen(id).weapon == Hammer){
                if (is_weapon(p_b)){
                    mapa_long[m_d[p_a.i][p_a.j]] += 100000.0;
                    found = true;
                    break;
                }
                else if (cell(p_b).bonus == Money){
                  mapa_long[m_d[p_a.i][p_a.j]] += 50.0 / m_v[p_a.i][p_a.j];
                }
                else if (cell(p_b).bonus == Food){
                  mapa_long[m_d[p_a.i][p_a.j]] += 100/ (citizen(id).life + m_v[p_a.i][p_a.j]);
                }
              }
            }
          } //Analisis
          {
            c_p.push(p_b);
            m_v[p_b.i][p_b.j] = 1;
            m_dist[p_b.i][p_b.j] = m_dist[p_a.i][p_a.j] + 1;
            m_d[p_b.i][p_b.j] = m_d[p_a.i][p_a.j];
          }
        }
     }
    {
      mejor_p = 0;
      map<Dir,float> :: iterator it2;
      it2 = mapa_long.begin();
      while(it2 != mapa_long.end())
      {
        if(it2->second > mejor_p)
        {
          mejor_d = it2->first;
          mejor_p = it2-> second;
        }
        ++it2;
      }
      if(mejor_p > 0)
      {
        d_f = mejor_d;
        return true;
      }
    }
    return false;
    }
  }
  bool busqueda_noche_w(Pos const &pos_inicial, Dir &d_f)
  {
    int id = cell(pos_inicial).id; //Id warrior
    queue<Pos> c_p; //Cola Pos
    mapa_dir m_d(board_rows(), vector<Dir>(board_cols())); //Mapa direcciones
    mapa_vis m_v(board_rows(), vector<int>(board_cols(), 0)); //Mapa visitados
    mapa_vis m_dist(board_rows(), vector<int>(board_cols(), 0)); //Mapa visitados
    m_d[pos_inicial.i][pos_inicial.j] = Down; //En las 4 primeras dir asigno la direccion d
    m_v[pos_inicial.i][pos_inicial.j] = 1;


    map <Dir,int> mapa_short; //Mapa con puntuaciones a cada direccion
    Dir mejor_d; //Mejor direccion despues de analizar
    int mejor_p = 0; //Mejor puntuacion despues de analizar

    //1) Miramos alrededor cercano
    {
    random_shuffle(dirs.begin(),dirs.end());
    for(Dir d : dirs) //Por cada direccion
    {
      Pos p_b = pos_inicial + d;
      if(!pos_ok(p_b)) continue;
      if(!requisitos_barricadas(p_b)) continue; //Si la posicion no es correcta O no se cumple el requisito de las barricadas -->
      if(citizen(id).type == Warrior){ //Si eres warrior
        if (citizen(id).weapon == Bazooka){
          if (es_enemigo(cell(p_b).id)){
            mapa_short[d] = 10;
            break;
          }
          else if (is_weapon(p_b)){
            mapa_short[d] = 5;
          }
          else if (cell(p_b).bonus == Money){
            mapa_short[d] = 3;
          }
          else if (cell(p_b).bonus == Food){
            mapa_short[d] = 10 - (citizen(id).life/10.0);
          }
        }
        else if (citizen(id).weapon == Gun){
          if (es_enemigo(cell(p_b).id) and citizen(cell(p_b).id).weapon != Bazooka){
            mapa_short[d] = 10;
            break;
          }
          else if (is_weapon(p_b)){
            if (cell(p_b).weapon == Bazooka){
                mapa_short[d] = 10;
                break;
            }
            mapa_short[d] = 5;
          }
          else if (cell(p_b).bonus == Money){
            mapa_short[d] = 3;
          }
          else if (cell(p_b).bonus == Food){
            mapa_short[d] = 10 - (citizen(id).life/15.0);
          }
        }
        else if (citizen(id).weapon == Hammer){
          if (es_enemigo(cell(p_b).id) and (citizen(cell(p_b).id).weapon == NoWeapon or citizen(cell(p_b).id).weapon == Hammer)){
            mapa_short[d] = 10;
            break;
          }
          else if (is_weapon(p_b)){
              mapa_short[d] = 10;
              break;
          }
          else if (cell(p_b).bonus == Money){
            mapa_short[d] = 3;
          }
          else if (cell(p_b).bonus == Food){
            mapa_short[d] = 10 - (citizen(id).life/20.0);
          }
        }
      }
      if(requisitos_totales(p_b)) //Si no hay persona ni barricadas
        {
          c_p.push(p_b);
          m_d[p_b.i][p_b.j] = d;
          m_dist[p_b.i][p_b.j] = 1;
        }
    }
    //Analisis direcciones
    {
      map <Dir,int>::iterator it = mapa_short.begin();
      while(it != mapa_short.end())
      {
        if(it->second > mejor_p)
        {
          mejor_d = it->first;
          mejor_p = it-> second;
        }
        ++it;
      }
      if(mejor_p > 0)
      {
        d_f = mejor_d;
        return true;
      }
    }
    }

//------------------------------------------------------------------------------------------_//
    //2) Miramos el mapa entero
    map <Dir,float> mapa_long;
    bool found = false;
    {
    while(!c_p.empty() and !found) //Mientras la cola no este vacia o NO encontrado algo interesante
    {
            random_shuffle(dirs.begin(),dirs.end());
      Pos p_a = c_p.front();
      c_p.pop();
      for(Dir d : dirs)
      {
        Pos p_b = p_a + d;
        if(!pos_ok(p_b)) continue;
        if(m_v[p_b.i][p_b.j] != 0) continue;
        if(!requisitos_barricadas(p_b))continue; //No miramos la posicion si hay alguna barricada
        if (citizen(id).type == Warrior){
          if (citizen(id).weapon == Bazooka){
            if (es_enemigo(cell(p_b).id)){
              mapa_long[m_d[p_a.i][p_a.j]] = 8;
              found = true;
              break;
            }
            else if (is_weapon(p_b)){
              mapa_long[m_d[p_a.i][p_a.j]] = 5;
            }
            else if (cell(p_b).bonus == Money){
              mapa_long[m_d[p_a.i][p_a.j]] = 3;
            }
            else if (cell(p_b).bonus == Food){
              mapa_long[m_d[p_a.i][p_a.j]] = 10 - (citizen(id).life/20.0);
            }
          }
          else if (citizen(id).weapon == Gun){
            if (es_enemigo(cell(p_b).id) and citizen(cell(p_b).id).weapon != Bazooka){
              mapa_long[m_d[p_a.i][p_a.j]] = 8;
              found = true;
              break;
            }
            else if (is_weapon(p_b)){
              if (cell(p_b).weapon == Bazooka){
                  mapa_long[m_d[p_a.i][p_a.j]] = 9;
                  found = true;
                  break;
              }
              mapa_long[d] = 5;
            }
            else if (cell(p_b).bonus == Money){
              mapa_long[m_d[p_a.i][p_a.j]] = 3;
            }
            else if (cell(p_b).bonus == Food){
              mapa_long[m_d[p_a.i][p_a.j]] = 10 - (citizen(id).life/20.0);
            }
          }
          else if (citizen(id).weapon == Hammer){
            if (es_enemigo(cell(p_b).id) and (citizen(cell(p_b).id).weapon == NoWeapon or citizen(cell(p_b).id).weapon == Hammer)){
              mapa_long[m_d[p_a.i][p_a.j]] = 6;
              found = true;
              break;
            }
            else if (is_weapon(p_b)){
              if (cell(p_b).weapon == Bazooka){
                  mapa_long[m_d[p_a.i][p_a.j]] = 9;
                  found = true;
                  break;
              }
              mapa_long[d] = 5;
            }
            else if (cell(p_b).bonus == Money){
              mapa_long[m_d[p_a.i][p_a.j]] = 3.0;
            }
            else if (cell(p_b).bonus == Food){
              mapa_long[m_d[p_a.i][p_a.j]] = 10.0 - (citizen(id).life/20.0);
            }
          }
        }
        c_p.push(p_b);
        m_v[p_b.i][p_b.j] = 1;
        m_dist[p_b.i][p_b.j] = m_dist[p_a.i][p_a.j] + 1;
        m_d[p_b.i][p_b.j] = m_d[p_a.i][p_a.j]; //En las otras asigno las anteriores
      }
    }
    {
      mejor_p = 0;
      map<Dir,float> :: iterator it2;
      it2 = mapa_long.begin();
      while(it2 != mapa_long.end())
      {
        if(it2->second > mejor_p)
        {
          mejor_d = it2->first;
          mejor_p = it2-> second;
        }
        ++it2;
      }
      if(mejor_p > 0)
      {
        d_f = mejor_d;
        return true;
      }
    }
    return false;
    }
  }
  void direccion_aleatoria(int const &id, Pos const &p_i)
  {
    int counter = 10;
    while (counter > 0){
      Dir random_dir = dirs[random(0,3)];
      if(pos_ok(p_i+random_dir) and requisitos_totales(p_i+random_dir)) {
        move(id, random_dir);
        return;
      }
      counter--;
    }
  }
  void w_d_action1()
  {
    // los guerreros se dedicaran a buscar armas/dinero
    vector<int> w = warriors(me());
    for(int id_w : w)
    {
      Pos p_w_inicial = citizen(id_w).pos;
      Dir d_f_w; //Direccion futura hacia donde se movera
      if(busqueda_dia_w(p_w_inicial, d_f_w)) move(id_w, d_f_w);
      else if(busqueda_centro(p_w_inicial, d_f_w)) move(id_w, d_f_w);
      else direccion_aleatoria(id_w, p_w_inicial);
    }
  }
  void w_n_action1()
  {
    //Checkeamos que alrededor no hay nadie primero --> si lo hay le pegamos
    vector<int> w = warriors(me());
    for(int id_w : w)
    {
      Pos p_w_inicial = citizen(id_w).pos;
      Dir d_f_w;
      if(busqueda_noche_w(p_w_inicial, d_f_w)) move(id_w, d_f_w);
      else if(busqueda_centro(p_w_inicial, d_f_w)) move(id_w, d_f_w);
      else direccion_aleatoria(id_w, p_w_inicial);
    }
  }
  void b_d_action1()
  {
    vector<int> b = builders(me());
    for(int id_b : b) //por cada guerrero que tengo
    {
      Pos p_b_inicial = citizen(id_b).pos;
      Dir d_f_b; //Direccion futura hacia donde se movera
      bool intr_encontrada = false;
      if(busqueda_dia_b(p_b_inicial, d_f_b, intr_encontrada)) move(id_b, d_f_b);
      else if(!intr_encontrada)
      {
        if(busqueda_centro(p_b_inicial, d_f_b)) move(id_b, d_f_b);
        else direccion_aleatoria(id_b, p_b_inicial);
      }
    }
  }
  virtual void play ()
  {
    if(is_day())
    {
      w_d_action1();
      b_d_action1();
    }
    else
    {
      w_n_action1();
      b_d_action1();
    }
  }
};
RegisterPlayer(PLAYER_NAME);
