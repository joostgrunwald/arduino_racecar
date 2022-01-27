using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MaxController
{
    public partial class Form1 : Form
    {
        //GLOBAL VARIABLES
        //becasue the speeds and brakes are motor controlled they are defined to be between 0 and 255.
        public int speed_forward = 0;
        public int speed_left = 0;
        public int speed_right = 0;
        public int brake_power = 0;
        public int gear = 0;

        public bool engineOn = false;
        public bool gasdown = false;
        public bool brakedown = false;
        public bool trotDown = false;

        //sound player initializing
        WMPLib.WindowsMediaPlayer wplayer = new WMPLib.WindowsMediaPlayer();

        public Form1()
        {
            InitializeComponent();
        }

        //used for rotating images
        public static Bitmap RotateImage(Image image, PointF offset, float angle)
        {
            if (image == null)
                throw new ArgumentNullException("image");

            //create a new empty bitmap to hold rotated image
            Bitmap rotatedBmp = new Bitmap(image.Width, image.Height);
            rotatedBmp.SetResolution(image.HorizontalResolution, image.VerticalResolution);

            //make a graphics object from the empty bitmap
            Graphics g = Graphics.FromImage(rotatedBmp);

            //Put the rotation point in the center of the image
            g.TranslateTransform(offset.X, offset.Y);

            //rotate the image
            g.RotateTransform(angle);

            //move the image back
            g.TranslateTransform(-offset.X, -offset.Y);

            //draw passed in image onto graphics object
            g.DrawImage(image, new PointF(0, 0));

            return rotatedBmp;
        }

        public void gearbox()
        {
            if (engineOn == true)
            {
                if ((speed_forward >= 100 && gear == 1) || (speed_forward >= 138 && gear == 2)
                    || (speed_forward >= 175 && gear == 3) || (speed_forward >= 212 && gear == 4))
                {
                    gear++;

                    //update image and bool
                    button3.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\trottle_pressed.png");
                    trotDown = true;

                    //make sound per gear
                    if (gear == 1) {
                        wplayer.URL = "C:\\Users\\joost\\source\\repos\\MaxController\\Sounds\\gear1.mp3";
                        wplayer.controls.play();
                    }
                    else if (gear == 2)
                    {
                        wplayer.URL = "C:\\Users\\joost\\source\\repos\\MaxController\\Sounds\\gear2.mp3";
                        wplayer.controls.play();
                    }
                    else if (gear == 3)
                    {
                        wplayer.URL = "C:\\Users\\joost\\source\\repos\\MaxController\\Sounds\\gear3.mp3";
                        wplayer.controls.play();
                    }
                    else if (gear == 4 || gear == 5)
                    {
                        wplayer.URL = "C:\\Users\\joost\\source\\repos\\MaxController\\Sounds\\gear45.mp3";
                        wplayer.controls.play();
                    }
                }
                else if ((speed_forward < 100 && gear == 2) || (speed_forward < 138 && gear == 3)
                    || (speed_forward < 175 && gear == 4) || (speed_forward < 212 & gear == 5))
                {
                    gear--;

                    //update image and bool
                    button3.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\trottle_pressed.png");
                    trotDown = true;
                }
            }
        }

        //Arrow key handler
        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            if (engineOn == true)
            {
                //capture up arrow key
                if (keyData == Keys.Up)
                {
                    if (gear != 0) { speed_forward += 5/gear; } //this way we accelerate slower
                    speed_left -= 3;
                    speed_right -= 3;
                    int actual_speed = 0;
                    int actual_speed_left = 0;
                    int actual_speed_right = 0;

                    //forward
                    if (speed_forward > 265) { speed_forward = 265; } //we set a maximum speed a bit above top shown speed, for better handling
                    if (speed_forward < 0) { speed_forward = 0; }
                    if (speed_forward >= 255) { actual_speed = 255; } else { actual_speed = speed_forward; }
                    label1.Text = actual_speed.ToString();

                    //left
                    if (speed_left > 265) { speed_left = 265; }
                    if (speed_left < 0) { speed_left = 0; }
                    if (speed_left >= 255) { actual_speed_left = 255; } else { actual_speed_left = speed_left; }
                    label3.Text = actual_speed_left.ToString();

                    //right
                    if (speed_right > 265) { speed_right = 265; }
                    if (speed_right < 0) { speed_right = 0; }
                    if (speed_right >= 255) { actual_speed_right = 255; } else { actual_speed_right = speed_right; }
                    label4.Text = actual_speed_right.ToString();

                    button1.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\gas_pressed.png");
                    button2.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\throttle.png");
                    gasdown = true;
                    brakedown = false;
                }
                //capture down arrow key
                if (keyData == Keys.Down)
                {
                    speed_forward -= 3;
                    int actual_speed = 0;
                    if (speed_forward < 0) { speed_forward = 0; } //we set a maximum speed a bit above top shown speed, for better handling
                    if (speed_forward >= 255) { actual_speed = 255; } else { actual_speed = speed_forward; }
                    label1.Text = actual_speed.ToString();
                    button2.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\brake_pressed.png");
                    button1.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\gas.png");
                    gasdown = false;
                    brakedown = true;
                    label5.Text = "Braking";

                }
                //capture left arrow key
                if (keyData == Keys.Left)
                {
                    if (gear > 0)
                    {
                        speed_left += 5;
                        speed_right = 0;
                    }

                    int actual_speed = 0;
                    int actual_speed_left = 0;
                    int actual_speed_right = 0;

                    //forward
                    if (speed_forward > 265) { speed_forward = 265; } //we set a maximum speed a bit above top shown speed, for better handling
                    if (speed_forward < 0) { speed_forward = 0; }
                    if (speed_forward >= 255) { actual_speed = 255; } else { actual_speed = speed_forward; }
                    label1.Text = actual_speed.ToString();

                    //left
                    if (speed_left > 265) { speed_left = 265; }
                    if (speed_left < 0) { speed_left = 0; }
                    if (speed_left >= 255) { actual_speed_left = 255; } else { actual_speed_left = speed_left; }
                    label3.Text = actual_speed_left.ToString();

                    //right
                    if (speed_right > 265) { speed_right = 265; }
                    if (speed_right < 0) { speed_right = 0; }
                    if (speed_right >= 255) { actual_speed_right = 255; } else { actual_speed_right = speed_right; }
                    label4.Text = actual_speed_right.ToString();
                }
                //capture right arrow key
                if (keyData == Keys.Right)
                {
                    if (gear > 0)
                    {
                        speed_right += 5;
                        speed_left = 0;
                    }

                    int actual_speed = 0;
                    int actual_speed_left = 0;
                    int actual_speed_right = 0;

                    //forward
                    if (speed_forward > 265) { speed_forward = 265; } //we set a maximum speed a bit above top shown speed, for better handling
                    if (speed_forward < 0) { speed_forward = 0; }
                    if (speed_forward >= 255) { actual_speed = 255; } else { actual_speed = speed_forward; }
                    label1.Text = actual_speed.ToString();

                    //left
                    if (speed_left > 265) { speed_left = 265; }
                    if (speed_left < 0) { speed_left = 0; }
                    if (speed_left >= 255) { actual_speed_left = 255; } else { actual_speed_left = speed_left; }
                    label3.Text = actual_speed_left.ToString();

                    //right
                    if (speed_right > 265) { speed_right = 265; }
                    if (speed_right < 0) { speed_right = 0; }
                    if (speed_right >= 255) { actual_speed_right = 255; } else { actual_speed_right = speed_right; }
                    label4.Text = actual_speed_right.ToString();
                }
                return base.ProcessCmdKey(ref msg, keyData);
            }
            return false;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //gas button clicked
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //brake button clicked
        }

        private void button3_Click(object sender, EventArgs e)
        {
            //throttle button clicked
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            //slowly making gass value lower
            speed_forward -= 2;
            speed_left -= 2;
            speed_right -= 2;

            int actual_speed = 0;
            int actual_speed_left = 0;
            int actual_speed_right = 0;

            //forward
            if (speed_forward > 265) { speed_forward = 265; } //we set a maximum speed a bit above top shown speed, for better handling
            if (speed_forward < 0) { speed_forward = 0; }
            if (speed_forward >= 255) { actual_speed = 255; } else { actual_speed = speed_forward; }
            label1.Text = actual_speed.ToString();
            if (speed_forward > 0) { label1.ForeColor = Color.Blue; } else { label1.ForeColor = Color.Red; }

            //left
            if (speed_left > 265) { speed_left = 265; }
            if (speed_left < 0) { speed_left = 0; }
            if (speed_left >= 255) { actual_speed_left = 255; } else { actual_speed_left = speed_left; }
            label3.Text = actual_speed_left.ToString();
            if (speed_left > 0) { label3.ForeColor = Color.Blue; } else { label3.ForeColor = Color.Red; }

            //right
            if (speed_right > 265) { speed_right = 265; }
            if (speed_right < 0) { speed_right = 0; }
            if (speed_right >= 255) { actual_speed_right = 255; } else { actual_speed_right = speed_right; }
            label4.Text = actual_speed_right.ToString();
            if (speed_right > 0) { label4.ForeColor = Color.Blue; } else { label4.ForeColor = Color.Red; }

            //updating the gearbox
            gearbox();
            if (engineOn == false) { gear = 0; }
            label2.Text = "Gear " + gear.ToString();
        }

        private void timer2_Tick(object sender, EventArgs e)
        {
            if (engineOn == true)
            {
                trotDown = false;
                if (gasdown == false)
                {
                    button1.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\gas.png");
                }
                if (brakedown == false)
                {
                    button2.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\throttle.png");
                    //clear braking label
                    label5.Text = "";
                }
                if (trotDown == false)
                {
                    button3.BackgroundImage = Image.FromFile(@"C:\\Users\\joost\\source\\repos\\MaxController\\Images\\trottle.png");
                }

                //reset (fixes flickering buttons)
                brakedown = false;
                gasdown = false;
            }
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void button4_Click(object sender, EventArgs e)
        {
            //engine on button which is used for both enabling and disabling the engine
            engineOn = !engineOn; //Flipping the engine property
            if (engineOn == true)
            {
                //if engine on play sound
                wplayer.URL = "C:\\Users\\joost\\source\\repos\\MaxController\\Sounds\\engine_star.mp3";
                wplayer.controls.play();
                gear = 1;
            }    
            else
            {
                //if engine off play sound
                wplayer.URL = "C:\\Users\\joost\\source\\repos\\MaxController\\Sounds\\car_off.mp3";
                wplayer.controls.play();
                gear = 0;
            }
        }

        private void timer3_Tick(object sender, EventArgs e)
        {
            //make sound per gear
            if (gear == 5)
            {
                wplayer.URL = "C:\\Users\\joost\\source\\repos\\MaxController\\Sounds\\gear45.mp3";
                wplayer.controls.play();
            }
        }

        private void label3_Click(object sender, EventArgs e)
        {

        }
    }
}
