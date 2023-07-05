import ctypes
import pathlib
import time
import os
import sys
from tkinter import *
from ctypes import *

LIGHT_RED = "#FF9393"
LIGHT_ORANGE = "#FFBE89"

##################################################
# Initialize C bindings
##################################################

# Load the shared library into ctypes
if sys.platform == 'win32':
  libname = os.path.join(pathlib.Path().absolute(), "libparticle.dll")
else : 
  libname = os.path.join(pathlib.Path().absolute(), "libparticle.so")
c_lib = ctypes.CDLL(libname)

# Define python equivalent class

class PARTICLE(Structure):
    _fields_ = [("position", c_float*2),
                ("next_pos", c_float*2),
                ("velocity", c_float*2),
                ("inv_mass", c_float),
                ("radius",   c_float),
                ("solid_id", c_int),
                ("draw_id",  c_int)]

class PLANECOLLIDER(Structure):
    _fields_ =  [("center", c_float*2),
                 ("normal", c_float*2)]

class SPHERECOLLIDER(Structure):
    _fields_ =  [("center", c_float*2),
                 ("radius", c_float)]

class CONTEXT(Structure):
    _fields_ = [("num_particles", c_int),
                ("particles", POINTER(PARTICLE) ),
                ("num_ground_plane", c_int),
                ("num_ground_sphere", c_int),
                ("ground_planes", POINTER(PLANECOLLIDER) ),
                ("ground_spheres", POINTER(SPHERECOLLIDER) ),
                ("water_surface", c_float)]

# ("pos", c_float*2) => fixed size array of two float


# Declare proper return types for methods (otherwise considered as c_int)
c_lib.initializeContext.restype = POINTER(CONTEXT) # return type of initializeContext is Context*
c_lib.getParticle.restype = PARTICLE
c_lib.getGroundPlaneCollider.restype = PLANECOLLIDER
c_lib.getGroundSphereCollider.restype = SPHERECOLLIDER
# WARNING : python parameter should be explicitly converted to proper c_type of not integer.
# If we already have a c_type (including the one deriving from Structure above)
# then the parameter can be passed as is.

##################################################
# Class managing the UI
##################################################

class ParticleUI :
    def __init__(self) :
        # create drawing context
        self.context = c_lib.initializeContext()
        self.width = 750
        self.height = 750

        # physical simulation will work in [-world_x_max,world_x_max] x [-world_y_max,world_y_max]
        self.world_x_max = 10
        self.world_y_max = 10
        # WARNING : the mappings assume world bounding box and canvas have the same ratio !

        self.window = Tk()

        # create simulation context...
        self.canvas = Canvas(self.window,width=self.width,height=self.height)
        self.canvas.pack()
        
        # used to fire particles
        self.aiming = False
        self.click_position = None
        self.viewfinder_circle = None
        self.viewfinder_line = None
        self.click = None # value in "Right" or "Left"
        
        self.canvas.create_rectangle(self.worldToView( (-self.world_x_max, self.context.contents.water_surface) ),
                                     self.worldToView( (self.world_x_max, -self.world_y_max) ),
                                     fill="#55A8FF", outline="")

        # Initialize drawing, only needed if the context is initialized with elements,
        # otherwise, see addParticle
        for i in range(self.context.contents.num_particles):
            sphere = c_lib.getParticle(self.context, i)
            draw_id = self.canvas.create_oval(*self.worldToView( (sphere.position[0]-sphere.radius,sphere.position[1]-sphere.radius) ),
                                              *self.worldToView( (sphere.position[0]+sphere.radius,sphere.position[1]+sphere.radius) ),
                                              fill="red", outline="")
            c_lib.setDrawId(self.context, i, draw_id)

        for i in range(self.context.contents.num_ground_sphere):
            sphere = c_lib.getGroundSphereCollider(self.context, i)
            self.canvas.create_oval(*self.worldToView( (sphere.center[0]-sphere.radius,sphere.center[1]-sphere.radius) ),
                                    *self.worldToView( (sphere.center[0]+sphere.radius,sphere.center[1]+sphere.radius) ),
                                    fill="green", outline="")
                                              
        for i in range(self.context.contents.num_ground_plane):
            plane = c_lib.getGroundPlaneCollider(self.context, i)
            self.canvas.create_polygon(*self.worldToView( (plane.center[0]-20*plane.normal[1],plane.center[1]+20*plane.normal[0]) ),
                                       *self.worldToView( (plane.center[0]+20*plane.normal[1],plane.center[1]-20*plane.normal[0]) ),
                                       *self.worldToView( (plane.center[0]+20*plane.normal[1]-20*plane.normal[0],plane.center[1]-20*plane.normal[0]-20*plane.normal[1]) ),
                                       *self.worldToView( (plane.center[0]-20*plane.normal[1]-20*plane.normal[0],plane.center[1]+20*plane.normal[0]-20*plane.normal[1]) ),
                                       fill="green", outline="")
        
        self.canvas.create_text(20, self.height - 40, text="Press left mouse button, drag and release to fire one particle.", fill="white", font=('Helvetica 10 bold'), anchor=W)
        self.canvas.create_text(20, self.height - 20, text="Press right mouse button, drag and release to fire a solid.", fill="white", font=('Helvetica 10 bold'), anchor=W)

        # Initialize Mouse and Key events
        self.canvas.bind("<ButtonPress-1>", lambda event: self.mouseLeftButtonPressCallback(event))
        self.canvas.bind("<ButtonRelease-1>", lambda event: self.mouseLeftButtonReleaseCallback(event))
        self.canvas.bind("<ButtonPress-3>", lambda event: self.mouseRightButtonPressCallback(event))
        self.canvas.bind("<ButtonRelease-3>", lambda event: self.mouseRightButtonReleaseCallback(event))
        self.canvas.bind("<Motion>", lambda event: self.updateAiming(event))
        self.window.bind("<Key>", lambda event: self.keyCallback(event)) # bind all key
        #self.window.bind("<Escape>", lambda event: self.enterCallback(event))
        self.window.protocol("WM_DELETE_WINDOW", self.enterCallback)
        # bind specific key overide default binding

    def launchSimulation(self) :
        # launch animation loop
        self.animate()
        # launch UI event loop
        self.window.mainloop()



    def animate(self) :
        """ animation loop """
        # APPLY PHYSICAL UPDATES HERE !
        for i in range(6) :
            c_lib.updatePhysicalSystem(self.context, c_float(0.016/float(6)), 1) #TODO can be called more than once..., just devise dt

        for i in range(self.context.contents.num_particles):
            sphere = c_lib.getParticle(self.context, i)
            self.canvas.coords(sphere.draw_id,
                               *self.worldToView( (sphere.position[0]-sphere.radius,sphere.position[1]-sphere.radius) ),
                               *self.worldToView( (sphere.position[0]+sphere.radius,sphere.position[1]+sphere.radius) ) )
        self.window.update()
        self.window.after(16, self.animate)

    # Conversion from worl space (simulation) to display space (tkinter canvas)
    def worldToView(self, world_pos) :
        return ( self.width *(0.5 + (world_pos[0]/self.world_x_max) * 0.5),
                 self.height *(0.5 - (world_pos[1]/self.world_y_max) * 0.5)) 
    def viewToWorld(self, view_pos) :
        return ( self.world_x_max * 2.0 * (view_pos[0]/self.width - 0.5) ,
                 self.world_y_max * 2.0 * (0.5-view_pos[1]/self.height))  

    def addParticle(self, screen_pos, radius, mass) :
        (x_world, y_world) = self.viewToWorld(screen_pos)
        (click_position_world_x, click_position_world_y) = self.viewToWorld((self.click_position.x, self.click_position.y))
        (speed_x_world, speed_y_world) = (3*(click_position_world_x - x_world), 3*(click_position_world_y - y_world))
        # min max bounding box in view coordinates, will be propertly initialized 
        # in the canvas oval after the first call to animate
        #b_min = self.worldToView( (x_world-radius,y_world-radius) )
        #b_max = self.worldToView( (x_world+radius,y_world+radius) )
        draw_id = self.canvas.create_oval(0,0,0,0,fill="red", outline="")
        c_lib.addParticle(self.context,
                        c_float(x_world), c_float(y_world),
                        c_float(speed_x_world), c_float(speed_y_world),
                        c_float(radius), c_float(mass),
                        draw_id)

    def addCoupledParticles(self, screen_pos, radius, mass) :
        (x_world, y_world) = self.viewToWorld(screen_pos)
        (click_position_world_x, click_position_world_y) = self.viewToWorld((self.click_position.x, self.click_position.y))
        (speed_x_world, speed_y_world) = (3*(click_position_world_x - x_world), 3*(click_position_world_y - y_world))
        # min max bounding box in view coordinates, will be propertly initialized 
        # in the canvas oval after the first call to animate
        #b_min = self.worldToView( (x_world-radius,y_world-radius) )
        #b_max = self.worldToView( (x_world+radius,y_world+radius) )
        draw_id1 = self.canvas.create_oval(0,0,0,0,fill="orange", outline="")
        draw_id2 = self.canvas.create_oval(0,0,0,0,fill="orange", outline="")
        draw_id3 = self.canvas.create_oval(0,0,0,0,fill="orange", outline="")
        draw_id4 = self.canvas.create_oval(0,0,0,0,fill="orange", outline="")
        c_lib.addCoupledParticles(self.context,
                        c_float(x_world), c_float(y_world),
                        c_float(speed_x_world), c_float(speed_y_world),
                        c_float(radius), c_float(mass),
                        draw_id1, draw_id2, draw_id3, draw_id4)

    # All mouse and key callbacks

    def mouseLeftButtonPressCallback(self, event):
        self.viewfinder_circle = self.canvas.create_oval((event.x - 10, event.y - 10), (event.x + 10, event.y + 10), fill=LIGHT_RED, outline="")
        self.aiming = True
        self.click_position = event
        self.click = "Left"

    def mouseLeftButtonReleaseCallback(self, event):
        if self.viewfinder_line: self.canvas.delete(self.viewfinder_line)
        if self.viewfinder_circle: self.canvas.delete(self.viewfinder_circle)
        self.aiming = False
        self.addParticle((event.x, event.y), 0.2, 1.0)

    def mouseRightButtonPressCallback(self, event):
        self.viewfinder_circle = self.canvas.create_oval((event.x - 10, event.y - 10), (event.x + 10, event.y + 10), fill=LIGHT_ORANGE, outline="")
        self.aiming = True
        self.click_position = event
        self.click = "Right"
        
    def mouseRightButtonReleaseCallback(self, event):
        if self.viewfinder_line: self.canvas.delete(self.viewfinder_line)
        if self.viewfinder_circle: self.canvas.delete(self.viewfinder_circle)
        self.aiming = False
        self.addCoupledParticles((event.x, event.y), 0.2, 2.5)

    def updateAiming(self, event):
        if self.aiming:
           if self.viewfinder_line:
              self.canvas.delete(self.viewfinder_line)
           if self.click == "Left":
              color = LIGHT_RED
           else:
              color = LIGHT_ORANGE
           self.viewfinder_line = self.canvas.create_line((self.click_position.x, self.click_position.y), (event.x, event.y), width=5, fill=color)
           self.canvas.coords(self.viewfinder_circle, event.x - 10, event.y - 10, event.x + 10,event.y + 10)

    def keyCallback(self, event):
        print(repr(event.char))

    def enterCallback(self):
        c_lib.freeAll(self.context)
        self.window.destroy()

gui = ParticleUI()
gui.launchSimulation()