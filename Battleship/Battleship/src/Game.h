﻿#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>
#include <iostream>
#include "Mapa.h"


/// La clase juego se encarga de usar todos los recursos graficos de OLC::PixelGameEngine
class Game : public olc::PixelGameEngine 
{
private:

    static const int _WordX = 10; /// Tamño de mi mapa eje x 
	static const int _WordY = 10; /// Tamño de mi mapa eje y

	/// olc::vi2d Son Vectores de 2 dimensiones con su x , y.
	olc::vi2d vWorldSize = { 10,10  };/// MAPA principal ... jugador humano.
	olc::vi2d vWorldWarSize = { _WordX , _WordY };/// MAPA segundario ... para jugador maquina por ahora...

	olc::vi2d isoTileSize = { 40 , 20 }; /// Tamaño en pixeles de cada una de mis Tiles isometricos.

	olc::vi2d vWorldOrigen = { 5 , 1 };/// Donde quiero que comience mi mapa en pantalla.
	olc::vi2d vWarOrigen = { 30 , 30 };

	/// Puntero que va a contener mis sprite a dibujar en consola.
	olc::Sprite* isoPng = nullptr;
	/// Supongo que voy a tener uno para cada barco y un contador para ir diferenciandolos.

	/// Puntero para contener mi matriz para crear un mundo 2D en un arreglo.
	int* pWorld = nullptr;
	int* pWarWorld = nullptr;
	int cntBarco = 1;
	/// Un vector lista para contener eventos para dar informacion en pantalla.
	std::list<std::string> listEvents;

	Mapa map;
	Barco GeneralBelgrano;

public:

	Game()
	{
		sAppName = "Game";
	}
	
		
		
	bool OnUserCreate() override
	{
		/// Cargo la direecion de mis sprites.
		isoPng = new olc::Sprite("assets/isodemo.png");

		/// Doy el tamaño de mi mapa al arreglo.
		pWorld = new int[vWorldSize.x * vWorldSize.y]{ 0 };
		pWarWorld = new int[vWorldWarSize.x * vWorldWarSize.y]{ 0 };
		
		GeneralBelgrano.setSize(3);
		/// Un for para ir cambiando de eventos.
		for (int i = 0; i < 10; i++)
			listEvents.push_back("");

		
		return true;
	}
	

	bool OnUserUpdate(float FpsTime) override 
	{
		
		// Pantalla completa en blanco.
		olc::PixelGameEngine::Clear(olc::WHITE);
		// Creo unas "Cordenadas" de mouse.
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };
		// Creo un array 2D que va referenciar a una "celda" en mi mapa.
		olc::vi2d vCell = { vMouse.x / isoTileSize.x, vMouse.y / isoTileSize.y };

		// Creo un offset de las celdas para que mi mouse pueda seleccionar correctamente cada celda
		// porque las casillas con 4 celdas no son reconocidas por el mouse ya que es un mapa isometrico.
		olc::vi2d vOffset = { vMouse.x % isoTileSize.x, vMouse.y % isoTileSize.y };

		// Para solucionar este problema hago que todas las cuadriculas en mi mapa tengan una textura default con 4 colores
		// esto va a hacer que cada ves que mi mouse reconozca uno de esos colores pueda definir un limite a mi cuadraro celecionado y asi
		// poder seleccionar cualquier cuadrado en el mapa
		olc::Pixel col = isoPng->GetPixel(3 * isoTileSize.x + vOffset.x, vOffset.y);

		/// Magia ╰( ͡° ͜ʖ ͡° )つ──☆*:・ﾟ
		/// Mentira fran despues comento bien lo que hace
		/// Basicamente Crea crea un array 2d de donde estas "seleccionadndo" en el mapa
		olc::vi2d vSelected =
		{
			(vCell.y - vWorldOrigen.y) + (vCell.x - vWorldOrigen.x),
			(vCell.y - vWorldOrigen.y) - (vCell.x - vWorldOrigen.x)
		};

		/// ╰( ͡° ͜ʖ ͡° )つ──☆*:・ﾟ X 2
		olc::vi2d vSelected2 =
		{
			(vCell.y - vWarOrigen.y) + (vCell.x - vWarOrigen.x),
			(vCell.y - vWarOrigen.y) - (vCell.x - vWarOrigen.x)
		};

		/// Donde coloques el cursor dibuja el sprite adyasente ( ͡° ͜ʖ ͡° )
		if (col == olc::RED) vSelected += {-1, +0};
		if (col == olc::BLUE) vSelected += {+0, -1};
		if (col == olc::GREEN) vSelected += {+0, +1};
		if (col == olc::YELLOW) vSelected += {+1, +0};

		/// Lo mismo pero para el mapa 2 ( ͡° ͜ʖ ͡° )
		if (col == olc::RED) vSelected2 += {-1, +0};
		if (col == olc::BLUE) vSelected2 += {+0, -1};
		if (col == olc::GREEN) vSelected2 += {+0, +1};
		if (col == olc::YELLOW) vSelected2 += {+1, +0};
		
		// Funcion Lambda para convertir "world" en cordenadas para la pantalla.
		auto ToScreen = [&](int x, int y)
		{
			return olc::vi2d
			{
				(vWorldOrigen.x * isoTileSize.x) + (x - y) * (isoTileSize.x / 2),
				(vWorldOrigen.y * isoTileSize.y) + (x + y) * (isoTileSize.y / 2)
			};
		};

		// Lo mismo para mi segundo mapa
		auto ToScreen2 = [&](int x, int y)
		{
			return olc::vi2d
			{
				(vWarOrigen.x * isoTileSize.x) + (x - y) * (isoTileSize.x / 2),
				(vWarOrigen.y * isoTileSize.y) + (x + y) * (isoTileSize.y / 2)
			};
		};

		
		// Da trasparencia al mundo.
		SetPixelMode(olc::Pixel::MASK);

	
		/// (0,0) es mi tope , dibujo desde el tope hasta el final para
		/// haer que mis tiles mas cercanas se dibujen al final.
		for (int y = 0; y < vWorldSize.y; y++)
		{
			for (int x = 0; x < vWorldSize.x; x++)
			{
				
				/// Convierte las celdas a cordenadas del mundo.
				olc::vi2d vWorld = ToScreen(x, y);

				switch (pWorld[y * vWorldSize.x + x])
				{
				case 0:
					// Invisble Tile                               
					DrawPartialSprite(vWorld.x, vWorld.y, isoPng, 1 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
					break;
				case 1:
					// Grass visible Tile
					DrawPartialSprite(vWorld.x, vWorld.y, isoPng, 2 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				}
			}
		}

		/// Es todo lo mismo para dibujar mi segundo mapa con unas cordenadas propias.
		for (int y = 0; y < vWorldWarSize.y; y++)
		{
			for (int x = 0; x < vWorldWarSize.x; x++)
			{
				olc::vi2d vWorld2 = ToScreen2(x, y); 

				switch (pWarWorld[y * vWorldWarSize.x + x])
				{
				case 0:
					// Invisble Tile                               
					DrawPartialSprite(vWorld2.x, vWorld2.y, isoPng, 1 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
					break;
				case 1:
					// Grass visible Tile
					DrawPartialSprite(vWorld2.x, vWorld2.y, isoPng, 2 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
					break;
				}

			}
		}


		auto AddEvent = [&](std::string s)
		{
			listEvents.push_back(s);
			listEvents.pop_front();
		};

		int nLog = 0;
		for (auto& s : listEvents)
		{
			// 8 + 20  Separacion entre textos        /// los 3 * 16
			DrawString(400, nLog * 8 + 20, s, olc::Pixel(nLog * -16, nLog * -16, nLog * -16), 1);
			nLog++;
		}

		//////////////////////////////// FIN PRIMITIVAS DEL JUEGO ///////////////////////////////////////////////

		/*Luego de dibujar las primitivas del mapa entra el jugador y bot con sus methodos*/
		/*El jugador coloca sus baracos al terminar se colocan barcos al bot*/
		/*El jugar dispara y recibe la informacion del bot*/
		/*Bot dispara y recibo la informacion*/
		/**/


		

		if (GetKey(olc::Key::E).bHeld)
		{
			SetPixelMode(olc::Pixel::NORMAL);
			Clear(olc::WHITE);
			vWarOrigen = { 16, 1 };
		}


		// Dibuja el sombreado amaralli en cada tile que selecciono.
		SetPixelMode(olc::Pixel::ALPHA);

		/// Convierto las cordenadas a espacio "real" en el mundo.
		olc::vi2d vSelectedWorld = ToScreen(vSelected.x, vSelected.y);
		olc::vi2d vSelectedWorld2 = ToScreen2(vSelected2.x, vSelected2.y); 

		
		if (GetMouse(0).bPressed)
		{
			if (map.insertShip(vSelected.x, vSelected.y, GeneralBelgrano))//map.ValidPlacement(vWorldSize.x, vWorldSize.y, D_UP, GeneralBelgrano, pWorld)
			{   
				switch (cntBarco)
				{
				case 1:
					++pWorld[vSelected.y * vWorldSize.x + vSelected.x] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 1] %= 2;
					cntBarco++;
					break;

				case 2:
					++pWorld[vSelected.y * vWorldSize.x + vSelected.x] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 1] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 2] %= 2;
					cntBarco++;
					break;

				case 3:
					++pWorld[vSelected.y * vWorldSize.x + vSelected.x] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 1] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 2] %= 2;
					cntBarco++;
					break;

				case 4:
					++pWorld[vSelected.y * vWorldSize.x + vSelected.x] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 1] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 2] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 3] %= 2;
					cntBarco++;
					break;
				
				case 5:
					++pWorld[vSelected.y * vWorldSize.x + vSelected.x] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 1] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 2] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 3] %= 2;
					++pWorld[(vSelected.y * vWorldSize.x + vSelected.x) + 4] %= 2;
					cntBarco++;
					break;

				default:
					AddEvent("Todos los barcos en mapa");
					break;
				}
			}else{
				AddEvent("Fuera del mapa");
			}
			
		}


		/// Datos para el mapa 2
		if (GetMouse(0).bPressed)
		{
			if (vSelected2.x >= 0 && vSelected2.x < vWorldWarSize.x && vSelected2.y >= 0 && vSelected2.y < vWorldWarSize.y)
			{
				++pWarWorld[vSelected2.y * vWorldWarSize.x + vSelected2.x] %= 2;
				++pWarWorld[(vSelected2.y * vWorldWarSize.x + vSelected2.x) + 1] %= 2;
			}
		}

		// Doy la textura de seleccion para cada barco y el mundo
		if (vMouse.x && vMouse.y != NULL) /// size del barquito y rotazion 
		{
			switch (cntBarco)
			{
			case 1: // horizontal
				DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 20, vSelectedWorld.y + 10, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				break;
			case 2:
				DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 20, vSelectedWorld.y + 10, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 40, vSelectedWorld.y + 20, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				break;
			case 3:
				DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 20, vSelectedWorld.y + 10, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 40, vSelectedWorld.y + 20, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				break;
			case 4:
				DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 20, vSelectedWorld.y + 10, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 40, vSelectedWorld.y + 20, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 60, vSelectedWorld.y + 30, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				break;
			case 5:
				DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 20, vSelectedWorld.y + 10, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 40, vSelectedWorld.y + 20, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 60, vSelectedWorld.y + 30, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				DrawPartialSprite(vSelectedWorld.x + 80, vSelectedWorld.y + 40, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				break;
			default:
				DrawPartialSprite(vSelectedWorld.x, vSelectedWorld.y, isoPng, 0 * isoTileSize.x, 0, isoTileSize.x, isoTileSize.y);
				break;
			}


			//X +20  ----- Y + 10 --- o --- X -20 Y -10 desde el centro   VERTICAL
			//DrawPartialSprite(vSelectedWorld.x + 20, vSelectedWorld.y + 10, sprIsom, 0 * vTileSize.x, 0, vTileSize.x, vTileSize.y);

			 // X - 20  ---- Y + 10 HORIZONTAL 
			//DrawPartialSprite(vSelectedWorld.x - 20, vSelectedWorld.y + 10, sprIsom, 0 * vTileSize.x, 0, vTileSize.x, vTileSize.y);

			//DrawPartialSprite(vSelectedWorld.x + 40, vSelectedWorld.y + 20, sprIsom, 0 * vTileSize.x, 0, vTileSize.x, vTileSize.y);
		}



		///////////////// DIBUJO DATOS AL MAPA /////////////////////////////////////////////////////////////////



		// Go back to normal drawing with no expected transparency
		SetPixelMode(olc::Pixel::NORMAL);

		// Draw Debug Info
		DrawString(4, 4, "Mouse   : " + std::to_string(vMouse.x) + ", " + std::to_string(vMouse.y), olc::BLACK);
		DrawString(4, 14, "Cell    : " + std::to_string(vCell.x) + ", " + std::to_string(vCell.y), olc::BLACK);

		if (vSelected.x >= 0 && vSelected.x < vWorldSize.x && vSelected.y >= 0 && vSelected.y < vWorldSize.y)
		{
			DrawString(4, 24, "Mapa[1] X: " + std::to_string(vSelected.x) + ",Y: " + std::to_string(vSelected.y), olc::BLACK);

		}
		else if (vSelected2.x >= 0 && vSelected2.x < vWorldWarSize.x && vSelected2.y >= 0 && vSelected2.y < vWorldWarSize.y)
		{
			DrawString(4, 24, "Mapa[2] X: " + std::to_string(vSelected2.x) + ",Y: " + std::to_string(vSelected2.y), olc::BLACK);
		}
		else {

			DrawString(4, 24, "Mapa [1]: Fuera del mapa ", olc::BLACK);
			DrawString(4, 34, "Mapa [2]: Fuera del mapa ", olc::BLACK);
		}


		return true;
	}

	                                     
	
	void Exit(); /// Metodo para salir del juego pulsando una tecla.

};
