int R, G, B;

void updateStrip()
{
  for (int i = 0; i < NUMws2812fx; i++)
    ws2812fx.setPixelColor(i, ws2812fx.Color(R, G, B));

  ws2812fx.show();
}


BLYNK_WRITE(V1)
{

  G = param[0].asInt();
  R = param[1].asInt();
  B = param[2].asInt();
  updateStrip();
}

BLYNK_WRITE(V2)
{
  ws2812fx.setBrightness(param[0].asInt());
  ws2812fx.show();
}

int mode = -1;
BLYNK_WRITE(V3)
{
  mode = (param[0].asInt());
}
