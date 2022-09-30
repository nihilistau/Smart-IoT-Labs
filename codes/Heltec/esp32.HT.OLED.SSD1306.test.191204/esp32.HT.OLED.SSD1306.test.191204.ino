//inclure la bibiliothèque de l'afficheur Oled
#include "SSD1306.h" 

// Initialiser l'oled en utilisant la bibliothèque de gestion de l'I2C ( Adresse I2C, pin SDA, pin SCL)
SSD1306 oled(0x3c,4, 15);  // 4,15 or 21, 22, 0x3c


void setup() {

   //Initialiser l'afficheur oled (sinon ne fonctionne pas)
    pinMode(16,OUTPUT);
    digitalWrite(16, LOW); // set  GPIO16 low to reset OLED
    delay(50); 
    digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 to high

   // Initialiser interface de l'OLED
   oled.init();

   //Inverser le sens d'affichage de l'oled. Mettre les 2 instructions sinon problème.
   oled.flipScreenVertically();
    oled.setFont(ArialMT_Plain_10);

}//fin setup

//Afficher taille de caractère différente
void drawFontFaceDemo() {

   //Aligner le texte
   oled.setTextAlignment(TEXT_ALIGN_LEFT);
    //taille des caractères
   oled.setFont(ArialMT_Plain_10);
    //position et texte à afficher
   oled.drawString(0, 0, "STI2D SIN");
    oled.setFont(ArialMT_Plain_16);
    oled.drawString(0, 10, "STI2D SIN");
    oled.setFont(ArialMT_Plain_24);
    oled.drawString(0, 26, "STI2D SIN");
}

//texte justifier
void drawTextFlowDemo() {

   oled.setFont(ArialMT_Plain_10);
    oled.setTextAlignment(TEXT_ALIGN_LEFT);
    oled.drawStringMaxWidth(0, 0, 128,
    "SmartComputerLab\n la meilleure plateforme IoT pour les ingenieurs et les developpeurs" );
}

//Alignement des textes
void drawTextAlignmentDemo() {

    oled.setFont(ArialMT_Plain_10);

   // The coordinates define the left starting point of the text
    oled.setTextAlignment(TEXT_ALIGN_LEFT);
    oled.drawString(0, 10, "Left aligned (0,10)");

   // The coordinates define the center of the text
   oled.setTextAlignment(TEXT_ALIGN_CENTER);
    oled.drawString(64, 22, "Center aligned (64,22)");

   // The coordinates define the right end of the text
    oled.setTextAlignment(TEXT_ALIGN_RIGHT);
    oled.drawString(128, 33, "Right aligned (128,33)");
}

void loop() {
    // efface l'oled
   oled.clear();

   // Appelle la fonction pour dessiner
    drawTextAlignmentDemo();

   //affiche sur l'oled -> écrit dans le buffer de l'oled
   oled.display();

   delay(2000);

   //Appelle le texte suivant
    oled.clear();
    drawFontFaceDemo();
   oled.display();

   delay(2000);


    //Appelle le texte suivant
   oled.clear();
    drawTextFlowDemo();
   oled.display();

   delay(2000);
}//fin loop
