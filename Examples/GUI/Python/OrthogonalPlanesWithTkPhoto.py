import vtk
from vtk import *
import Tkinter
from Tkinter import *
import sys, os
import vtk.tk
import vtk.tk.vtkLoadPythonTkWidgets
import vtk.tk.vtkTkImageViewerWidget
from vtk.tk.vtkTkPhotoImage import *
from vtk.util.misc import *


class SampleViewer:
    def __init__ ( self ):
        self.Tk = Tk = Tkinter.Tk();
        Tk.title ( 'Python Version of vtkImageDataToTkPhoto' );

        # Image pipeline
        reader = vtkVolume16Reader ()
        reader.SetDataDimensions ( 64, 64 )
        reader.SetDataByteOrderToLittleEndian ( )
        reader.SetFilePrefix ( vtkGetDataRoot() + '/Data/headsq/quarter'  )
        reader.SetImageRange ( 1, 93 )
        reader.SetDataSpacing ( 3.2, 3.2, 1.5 )
        reader.Update ()

        # Make the image a little bigger
        resample = vtkImageResample ()
        resample.SetInput ( reader.GetOutput() )
        resample.SetAxisMagnificationFactor ( 0, 2 )
        resample.SetAxisMagnificationFactor ( 1, 2 )
        resample.SetAxisMagnificationFactor ( 2, 1 )

        self.cast = cast = vtkImageShiftScale ()
        cast.SetInput ( resample.GetOutput() )
        cast.SetOutputScalarTypeToUnsignedChar( )
        cast.ClampOverflowOn ()
        cast.Update ()
        l,h = reader.GetOutput().GetScalarRange()

        # Create the three orthogonal views
        
        tphoto = self.tphoto = self.tphoto = vtkTkPhotoImage ();
        cphoto = self.cphoto = vtkTkPhotoImage ();
        sphoto = self.sphoto = vtkTkPhotoImage ();
        self.Position = [0, 0, 0]

        w = self.TransverseLabelWidget = Label ( Tk, image = tphoto )
        w.grid ( row = 0, column = 0 )
        w.bind ( "<Button1-Motion>", lambda e, i=tphoto, o='transverse', s=self: s.Motion ( e, i, o ) )
        w = Label ( Tk, image = cphoto )
        w.grid ( row = 1, column = 0 )
        w.bind ( "<Button1-Motion>", lambda e, i=cphoto, o='coronal', s=self: s.Motion ( e, i, o ) )
        w = Label ( Tk, image = sphoto )
        w.grid ( row = 0, column = 1 )
        w.bind ( "<Button1-Motion>", lambda e, i=sphoto, o='sagittal', s=self: s.Motion ( e, i, o ) )
        w = self.WindowWidget = Scale ( Tk, label='Window', orient='horizontal', from_=1, to=(h-l)/2, command = self.SetWindowLevel )
        w = self.LevelWidget = Scale ( Tk, label='Level', orient='horizontal', from_=l, to=h, command=self.SetWindowLevel )
        self.WindowWidget.grid ( row=2, columnspan=2, sticky='ew' )
        self.LevelWidget.grid ( row=3, columnspan=2, sticky='ew' );
        self.WindowWidget.set ( 1370 );
        self.LevelWidget.set ( 1268 );

        w = self.LabelWidget = Label ( Tk, bd=2, relief='raised' )
        w.grid ( row=4, columnspan=2, sticky='ew' )


    def Motion ( self, event, image, orientation ):
        w = image.width();
        h = image.height()
        if orientation == 'transverse':
            self.Position[0] = event.x
            self.Position[1] = h - event.y - 1
        if orientation == 'coronal':
            self.Position[0] = event.x;
            self.Position[2] = event.y
        if orientation == 'sagittal':
            self.Position[1] = w - event.x - 1
            self.Position[2] = event.y
        self.LabelWidget.configure ( text = "Position: %d, %d, %d" % tuple ( self.Position ) )
        self.SetImages()

    def SetWindowLevel ( self, event ):
        Window = self.WindowWidget.get()
        Level = self.LevelWidget.get()
        self.cast.SetScale ( 255.0 / Window )
        self.cast.SetShift ( Window / 2.0 - Level )
        self.SetImages()
        
    def SetImages ( self ):
        self.tphoto.PutImageSlice ( self.cast.GetOutput(), self.Position[2], 'transverse' )
        self.sphoto.PutImageSlice ( self.cast.GetOutput(), self.Position[0], 'sagittal' )
        self.cphoto.PutImageSlice ( self.cast.GetOutput(), self.Position[1], 'coronal' )



if __name__ == '__main__':
    S = SampleViewer()
    S.Tk.mainloop()
