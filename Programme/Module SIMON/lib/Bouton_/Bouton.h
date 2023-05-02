


class Bouton{

    public:
        Bouton();

        void begin(int pin,bool type_bt,int delay_click,int delay_press,int delay_rebond);

        bool isCliked();
        bool isPressed();
        void read_Bt();
        int etat();
    
    private:
        bool timer(int delay);
        void reset();  
        int d_read(); 
        void timer_reset();

        int PIN;
        unsigned int TIME_BT;
        bool TYPE;
        int STATE;

        int DELAY_CLICK;
        int DELAY_PRESS;
        int DELAY_REBOND;
        int DELAY_RESET;
};