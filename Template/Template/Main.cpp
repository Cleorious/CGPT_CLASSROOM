// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "Camera.h"
#include <MMSystem.h>

//----------------------------------------------------------------------
//Define the screen resolution and keyboard macros
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

//----------------------------------------------------------------------
// include the Direct3D Library files
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "winmm.lib")

//----------------------------------------------------------------------
// GLOBAL DECLARATIONS
LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;

LPDIRECT3DVERTEXBUFFER9 v_buffer[3];	//pointers to vertex buffers
LPDIRECT3DINDEXBUFFER9 i_buffer[3];		//pointers to index buffers
LPDIRECT3DTEXTURE9 texture[3];			//pointers to our textures

//----------------------------------------------------------------------
//skybox
LPDIRECT3DVERTEXBUFFER9 skybox_vBuffer;	//pointer to our skybox vertex buffer
LPDIRECT3DINDEXBUFFER9 skybox_iBuffer;	//pointer to our skybox index buffer
LPDIRECT3DTEXTURE9 skytexture[6];		//pointer to our skybox textures

Camera ThisCamera;						//declare first person camera class

//----------------------------------------------------------------------
//fonts
LPD3DXFONT d3dfont;						//pointer to our D3DFont

//----------------------------------------------------------------------
//text settings
int characterWidth	=	4;
LPCSTR text1		=	"Box 1";
int CharCount1		=	5;
LPCSTR text2		=	"Box 2";
int CharCount2		=	5;

//----------------------------------------------------------------------
// FUNCTION PROTOTYPES
void initD3D(HWND hWnd);	//Direct3D initializations
void init_plane(void);		//Initialize our plane
void init_skybox(void);		//Initialize our skybox
void init_light(void);		//Sets up the light and the material
void render_frame(void);	//Begin scene rendering and end
void cleanD3D(void);		//Release all pointers from memory

//----------------------------------------------------------------------
struct CUSTOMVERTEX {FLOAT X, Y, Z; D3DVECTOR NORMAL; FLOAT U, V;};
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//----------------------------------------------------------------------
// ENTRY POINT FOR WINDOWS PROGRAM
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "WindowClass";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(NULL, "WindowClass", "My Assignment Template",
                          WS_EX_TOPMOST | WS_POPUP,    // fullscreen values
                          0, 0,    // the starting x and y positions should be 0
						  SCREEN_WIDTH, SCREEN_HEIGHT,
                          NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    //set up and initialize Direct3D
    initD3D(hWnd);
	
	//sets the background music
	//PlaySound(TEXT("BGMFileNameHere.wav"), NULL, SND_ASYNC | SND_LOOP);

    // enter the main loop:

    MSG msg;

	//our real time loop
    while(TRUE)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(msg.message == WM_QUIT)
            break;

        //real-time rendering
		render_frame();

        // check the 'escape' key, if ESC is pressed, END program
        if(KEY_DOWN(VK_ESCAPE))
            PostMessage(hWnd, WM_DESTROY, 0, 0);

		//check for first person camera control AWSD and arrow keys
		if(KEY_DOWN(VK_UP) || KEY_DOWN('W'))
		{
			ThisCamera.walk(0.5f);
		}
	
		if(KEY_DOWN(VK_DOWN) || KEY_DOWN('S'))
		{
			ThisCamera.walk(-0.5f);
		}

		if(KEY_DOWN('A'))
		{
			ThisCamera.strafe(-0.5f);
		}
	
		if(KEY_DOWN('D'))
		{
			ThisCamera.strafe(0.5f);
		}

		if(KEY_DOWN(VK_LEFT))
		{
			ThisCamera.yaw(-0.05f);
		}

		if(KEY_DOWN(VK_RIGHT))
		{
			ThisCamera.yaw(0.05f);
		}		
    }

    // clean up DirectX and COM
    cleanD3D();

    return msg.wParam;
}

//----------------------------------------------------------------------
// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}

//----------------------------------------------------------------------
// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferWidth = SCREEN_WIDTH;
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    //create a device class using this information and the info from the d3dpp stuct
    d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &d3ddev);
	
	//create our font settinngs
	D3DXCreateFont(d3ddev,		//the D3D Device
		30,				//font height of 30
		0,				//default font width
		FW_NORMAL,		//font weight
		1,				//not using Miplevels
		false,			//italic/non-italic
		DEFAULT_CHARSET,//default character set
		OUT_CHARACTER_PRECIS,	//default OutputPrecision
		DEFAULT_QUALITY,		//default Quality)
		DEFAULT_PITCH | FF_DONTCARE,	//default Pitch and Family
		"Century Gothic",				//facename
		&d3dfont);						//our pointer to the font

  	init_plane();   //call the function to initialize our plane
	init_skybox();	//call the function to initialize our skybox
    init_light();   //call the function to initialize the light and material

    d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);							//turn on the 3D lighting
    d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);							//turn on the z-buffer
	d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);					//turn off culling
    d3ddev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(150, 150, 150));		//set the ambient light
	//d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);			//turn this on for debugging purposes to view your vertices in wireframe mode
}

//----------------------------------------------------------------------
// this is the function that puts the plane untransformed vertices information into video RAM
void init_plane(void)
{    
	D3DXCreateTextureFromFile(d3ddev,    // the Direct3D device
                              "floor.png",    // the filename of the texture
                              &texture[0]);    // the address of the texture storage

	//plane vertices
    CUSTOMVERTEX plane[] =
    {
		{ -100.0f, -1.0f, -100.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f, },    
        { 100.0f, -1.0f, -100.0f,  0.0f, -1.0f, 0.0f,  2.0f, 0.0f, },
        { -100.0f, -1.0f, 100.0f,  0.0f, -1.0f, 0.0f,  0.0f, 2.0f, },
        { 100.0f, -1.0f, 100.0f,  0.0f, -1.0f, 0.0f,  2.0f, 2.0f, },
    };

	// create a vertex buffer interface called v_buffer
    d3ddev->CreateVertexBuffer(4*sizeof(CUSTOMVERTEX),
                               0,
                               CUSTOMFVF,
                               D3DPOOL_MANAGED,
                               &v_buffer[0],
                               NULL);

	void* pVoid;	//a void pointer for use
    
	//lock v_buffer and load the vertices into it
    v_buffer[0]->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, plane, sizeof(plane));
    v_buffer[0]->Unlock();
}

void init_skybox(void)
{
	D3DXCreateTextureFromFile(d3ddev,
		"SkyBox_Front.jpg",
		&skytexture[0]);

	D3DXCreateTextureFromFile(d3ddev,
		"SkyBox_Back.jpg",
		&skytexture[1]);

	D3DXCreateTextureFromFile(d3ddev,
		"SkyBox_Left.jpg",
		&skytexture[2]);

	D3DXCreateTextureFromFile(d3ddev,
		"SkyBox_Right.jpg",
		&skytexture[3]);

	D3DXCreateTextureFromFile(d3ddev,
		"SkyBox_Top.jpg",
		&skytexture[4]);

	D3DXCreateTextureFromFile(d3ddev,
		"SkyBox_Bottom.jpg",
		&skytexture[5]);

	// create the vertices using the CUSTOMVERTEX
	struct CUSTOMVERTEX skybox_vertices[] =
	{
		{ -100.0f, -100.0f, 100.0f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, },		// front quad
		{ -100.0f, 100.0f, 100.0f, 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, },
		{ 100.0f, -100.0f, 100.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f, },
		{ 100.0f, 100.0f, 100.0f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, },

		{ 100.0f, -100.0f, -100.0f, 0.0f, 0.0f, -1.0f,  0.0f, 1.0f, },		// back quad
		{ 100.0f, 100.0f, -100.0f, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f, },
		{ -100.0f, -100.0f, -100.0f, 0.0f, 0.0f, -1.0f,  1.0f, 1.0f, },
		{ -100.0f, 100.0f, -100.0f, 0.0f, 0.0f, -1.0f,  1.0f, 0.0f, },

		{ -100.0f, -100.0f, -100.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, },		// left quad
		{ -100.0f, 100.0f, -100.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, },
		{ -100.0f, -100.0f, 100.0f, 0.0f, 1.0f, 0.0f,  1.0f, 1.0f, },
		{ -100.0f, 100.0f, 100.0f, 0.0f, 1.0f, 0.0f,  1.0f, 0.0f, },

		{ 100.0f, -100.0f, 100.0f, 0.0f, -1.0f, 0.0f,  0.0f, 1.0f, },		// right quad
		{ 100.0f, 100.0f, 100.0f, 0.0f, -1.0f, 0.0f,  0.0f, 0.0f, },
		{ 100.0f, -100.0f, -100.0f, 0.0f, -1.0f, 0.0f,  1.0f, 1.0f, },
		{ 100.0f, 100.0f, -100.0f, 0.0f, -1.0f, 0.0f,  1.0f, 0.0f, },

		{ -100.0f, 100.0f, 100.0f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f, },		// top quad
		{ -100.0f, 100.0f, -100.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, },
		{ 100.0f, 100.0f, 100.0f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f, },
		{ 100.0f, 100.0f, -100.0f, 1.0f, 0.0f, 0.0f,  1.0f, 0.0f, },

		{ -100.0f, -100.0f, -100.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, },		// bottom quad
		{ -100.0f, -100.0f, 100.0f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, },
		{ 100.0f, -100.0f, -100.0f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f, },
		{ 100.0f, -100.0f, 100.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f, },
	};

	// create a vertex buffer interface called skybox_vBuffer
	d3ddev->CreateVertexBuffer(24*sizeof(CUSTOMVERTEX),
		0,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&skybox_vBuffer,
		NULL);

	VOID* pVoid;    // a void pointer

	// lock box_vBuffer and load the vertices into it
	skybox_vBuffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, skybox_vertices, sizeof(skybox_vertices));
	skybox_vBuffer->Unlock();

	// create the indices using an int array
	short skybox_indices[] =
	{
		0, 1, 2,    // side 1
		2, 1, 3,
		4, 5, 6,    // side 2
		6, 5, 7,
		8, 9, 10,    // side 3
		10, 9, 11,
		12, 13, 14,    // side 4
		14, 13, 15,
		16, 17, 18,    // side 5
		18, 17, 19,
		20, 21, 22,    // side 6
		22, 21, 23,
	};

	// create a index buffer interface called skybox_iBuffer
	d3ddev->CreateIndexBuffer(36*sizeof(short),
		0,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&skybox_iBuffer,
		NULL);

	// lock box_iBuffer and load the indices into it
	skybox_iBuffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, skybox_indices, sizeof(skybox_indices));
	skybox_iBuffer->Unlock(); 
}

//----------------------------------------------------------------------
// this is the function that sets up the lights and materials
void init_light(void)
{
	D3DLIGHT9 light;    // create the light struct
	D3DMATERIAL9 material;    // create the material struct

	ZeroMemory(&light, sizeof(light));    // clear out the light struct for use
	light.Type = D3DLIGHT_DIRECTIONAL;    // make the light type 'directional light'
	light.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);    // set the light's color
	light.Position = D3DXVECTOR3(0.0f, 10.0f, 0.0f);
	light.Direction = D3DXVECTOR3(-1.0f, -0.3f, -1.0f);

	d3ddev->SetLight(0, &light);    // send the light struct properties to light #0
	d3ddev->LightEnable(0, TRUE);    // turn on light #0

	ZeroMemory(&material, sizeof(D3DMATERIAL9));    // clear out the struct for use
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);    // set diffuse color to white
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);    // set ambient color to white

	d3ddev->SetMaterial(&material);    // set the globaly-used material to &material
}

//----------------------------------------------------------------------
// this is the function used to render a single frame
void render_frame(void)
{
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();

	// select which vertex format we are using
	d3ddev->SetFVF(CUSTOMFVF);

	//make our first text box
	static RECT textbox1;
	SetRect(&textbox1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
	d3dfont->DrawTextA(NULL,
		text1,
		CharCount1,
		&textbox1,
		DT_LEFT | DT_TOP,
		D3DCOLOR_XRGB(255, 255, 255));

	//make our second text box
	static RECT textbox2;
	SetRect(&textbox2, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
	d3dfont->DrawTextA(NULL,
		text2,
		CharCount2,
		&textbox2,
		DT_LEFT | DT_BOTTOM,
		D3DCOLOR_XRGB(255, 255, 255));

	//create our first person camera
	D3DXMATRIX cam;

	//set our camera view matrix
	ThisCamera.getViewMatrix(&cam);

	//set our view transform to camera view
	d3ddev->SetTransform(D3DTS_VIEW, &cam);

	//----------------------------------------------------------------------
	//enable this to set the view transform to a fixed camera
	//D3DXMATRIX matView;
	//D3DXMatrixLookAtLH(&matView,
	//&D3DXVECTOR3 (0.0f, 6.0f, 30),    // the camera position
	//&D3DXVECTOR3 (0.0f, 0.0f, 0.0f),      // the look-at position
	//&D3DXVECTOR3 (0.0f, 1.0f, 0.0f));    // the up direction
	//d3ddev->SetTransform(D3DTS_VIEW, &matView);
	//----------------------------------------------------------------------

	//set the projection transform
	D3DXMATRIX matProjection;
	D3DXMatrixPerspectiveFovLH(&matProjection,
		D3DXToRadian(45),
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT,
		1.0f,    // the near view-plane
		300.0f);    // the far view-plane

	//set our projection transform
	d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);

	//----------------------------------------------------------------------
	//Setting the plane
	//----------------------------------------------------------------------

	// select the plane vertex buffer to use
	d3ddev->SetStreamSource(0, v_buffer[0], 0, sizeof(CUSTOMVERTEX));
	
	// set the plane texture
	d3ddev->SetTexture(0, texture[0]);
	
	//draw the plane
	d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	//Setting the skybox
	//----------------------------------------------------------------------
	
	//select the skybox vertex buffer to render
	d3ddev->SetStreamSource(0, skybox_vBuffer, 0, sizeof(CUSTOMVERTEX));

	//select the skybox index buffer to render
	d3ddev->SetIndices(skybox_iBuffer);

	//draw the skybox
	for (int i = 0; i < 6; ++i)
	{
		// Set the texture for our skybox
		d3ddev->SetTexture(0, skytexture[i]);

		// Render the face (one strip per face from the vertex buffer)  There are 2 primitives per face.
		d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
	}
	//end skybox
	//----------------------------------------------------------------------

	d3ddev->EndScene();

	d3ddev->Present(NULL, NULL, NULL, NULL);
}

//----------------------------------------------------------------------
// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	for (int i = 0; i<3; i++)
	{
		if (v_buffer[i] != NULL)
			v_buffer[i]->Release();

		if (texture[i] != NULL)
			texture[i]->Release();    // close and release the texture	
	}

	for (int j = 0; j<3; j++)
	{
		if (i_buffer[j] != NULL)
			i_buffer[j]->Release();
	}

	for (int k = 0; k<6; k++)
	{
		if (skytexture[k] != NULL)
			skytexture[k]->Release();		// close and release the texture	
	}

	skybox_vBuffer->Release();
	skybox_iBuffer->Release();

	if (d3ddev != NULL)
		d3ddev->Release();

	if (d3d != NULL)
		d3d->Release();
}