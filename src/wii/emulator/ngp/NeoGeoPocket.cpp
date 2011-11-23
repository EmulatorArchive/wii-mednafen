#include "main.h"

#include "NeoGeoPocket.h"
#include "wii_util.h"
#include "wii_input.h"
#include "wii_hw_buttons.h"

#include "wii_mednafen.h"
#include "wii_mednafen_main.h"

static const ScreenSize defaultScreenSizes[] = 
{
  { { 320,         240        },  "1x"          },
  { { 320*2,       240*2      },  "2x"          },
  { { 320*2.973,   240*2.973  },  "Full screen" },
  { { 320*4,       240*2.973  },  "Fill screen" }
};

static const int defaultScreenSizesCount =
  sizeof( defaultScreenSizes ) / sizeof(ScreenSize);

NeoGeoPocket::NeoGeoPocket() : 
  Emulator( "ngp", "Neo Geo Pocket" ),
  m_configManager( *this ),
  m_dbManager( *this ),
  m_menuManager( *this ),
  m_gameLanguage( NGP_LANG_ENGLISH )
{
  // The emulator screen size
  m_emulatorScreenSize.w = 160;
  m_emulatorScreenSize.h = 152;

  // Set user screen sizes
  int defaultIndex = 2;
  m_screenSize.w = defaultScreenSizes[defaultIndex].r.w; 
  m_screenSize.h = defaultScreenSizes[defaultIndex].r.h; 
}

ConfigManager& NeoGeoPocket::getConfigManager()
{ 
  return m_configManager;
}

DatabaseManager& NeoGeoPocket::getDbManager()
{
  return m_dbManager;
}
 
MenuManager& NeoGeoPocket::getMenuManager()
{
  return m_menuManager;
}

void NeoGeoPocket::updateControls( bool isRapid )
{
  WPAD_ScanPads();
  PAD_ScanPads();

  // Check the state of the controllers
  u32 pressed = WPAD_ButtonsDown( 0 );
  u32 held = WPAD_ButtonsHeld( 0 );  
  u32 gcPressed = PAD_ButtonsDown( 0 );
  u32 gcHeld = PAD_ButtonsHeld( 0 );

  // Classic or Nunchuck?
  expansion_t exp;
  WPAD_Expansion( 0, &exp );          

  BOOL isClassic = ( exp.type == WPAD_EXP_CLASSIC );
  BOOL isNunchuk = ( exp.type == WPAD_EXP_NUNCHUK );

  // Mask off the Wiimote d-pad depending on whether a nunchuk
  // is connected. (Wiimote d-pad is left when nunchuk is not
  // connected, right when it is).
  u32 heldLeft = ( isNunchuk ? ( held & ~0x0F00 ) : held );
  u32 heldRight = ( !isNunchuk ? ( held & ~0x0F00 ) : held );

  // Analog for Wii controls
  float expX = wii_exp_analog_val( &exp, TRUE, FALSE );
  float expY = wii_exp_analog_val( &exp, FALSE, FALSE );
  float expRX = isClassic ? wii_exp_analog_val( &exp, TRUE, TRUE ) : 0;
  float expRY = isClassic ? wii_exp_analog_val( &exp, FALSE, TRUE ) : 0;

  // Analog for Gamecube controls
  s8 gcX = PAD_StickX( 0 );
  s8 gcY = PAD_StickY( 0 );
  s8 gcRX = PAD_SubStickX( 0 );
  s8 gcRY = PAD_SubStickY( 0 );

  // Check for home
  if( ( pressed & WII_BUTTON_HOME ) ||
    ( gcPressed & GC_BUTTON_HOME ) ||
    wii_hw_button )
  {
    GameThreadRun = 0;
  }

  u16 result = 0;

  //
  // Mapped buttons
  //
  
  StandardDbEntry* entry = (StandardDbEntry*)getDbManager().getEntry();

  for( int i = 0; i < NGP_BUTTON_COUNT; i++ )
  {
    if( ( held &
          ( ( isClassic ? 
                entry->appliedButtonMap[ 
                  WII_CONTROLLER_CLASSIC ][ i ] : 0 ) |
            ( isNunchuk ?
                entry->appliedButtonMap[
                  WII_CONTROLLER_CHUK ][ i ] :
                entry->appliedButtonMap[
                  WII_CONTROLLER_MOTE ][ i ] ) ) ) ||
        ( gcHeld &
            entry->appliedButtonMap[
              WII_CONTROLLER_CUBE ][ i ] ) )
    {
      u32 val = NeoGeoPocketDbManager::NGP_BUTTONS[ i ].button;
      if( !( val & BTN_RAPID ) || isRapid )
      {
        result |= ( val & 0xFFFF );
      }
    }
  }    

  if( wii_digital_right( !isNunchuk, isClassic, heldLeft ) ||
      ( gcHeld & GC_BUTTON_RIGHT ) ||
      wii_analog_right( expX, gcX ) )
    result|=NGP_RIGHT;

  if( wii_digital_left( !isNunchuk, isClassic, heldLeft ) || 
      ( gcHeld & GC_BUTTON_LEFT ) ||                       
      wii_analog_left( expX, gcX ) )
    result|=NGP_LEFT;

  if( wii_digital_up( !isNunchuk, isClassic, heldLeft ) || 
      ( gcHeld & GC_BUTTON_UP ) ||
      wii_analog_up( expY, gcY ) )
    result|=NGP_UP;

  if( wii_digital_down( !isNunchuk, isClassic, heldLeft ) ||
      ( gcHeld & GC_BUTTON_DOWN ) ||
      wii_analog_down( expY, gcY ) )
    result|=NGP_DOWN;

  m_padData[0] = result;
}

void NeoGeoPocket::onPostLoad()
{
}

bool NeoGeoPocket::updateDebugText( 
  char* output, const char* defaultOutput, int len )
{
  return false;
}

bool NeoGeoPocket::isRotationSupported()
{
  return false;
}

int  NeoGeoPocket::getGameLanguage()
{
  return m_gameLanguage;
}

void  NeoGeoPocket::setGameLanguage( int lang )
{
  m_gameLanguage = lang;
}

const ScreenSize* NeoGeoPocket::getDefaultScreenSizes()
{
  return defaultScreenSizes;
}

int NeoGeoPocket::getDefaultScreenSizesCount()
{
  return defaultScreenSizesCount;
}

const ScreenSize* NeoGeoPocket::getDoubleStrikeScreenSize()
{
  return &defaultScreenSizes[1];
}