from AppShell import *


class TaskManagerPanel(AppShell):
    # Override class variables here
    appname = 'TaskManager Panel'
    frameWidth      = 300
    frameHeight     = 400
    usecommandarea = 0
    usestatusarea  = 0

    def __init__(self, taskMgr, parent = None, **kw):
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('title',       self.appname,       None),
            ('taskMgr',     None,            None)
            )
        self.defineoptions(kw, optiondefs)

        # Call superclass initialization function
        AppShell.__init__(self, parent = parent)
        
        self.initialiseoptions(TaskManagerPanel)

    def createInterface(self):
        # FILE MENU
        # Get a handle on the file menu so commands can be inserted
        # before quit item
        self.taskMgrWidget = TaskManagerWidget(
            self.interior(), self['taskMgr'])


    def onDestroy(self, event):
        self.ignore('TaskManager-setVerbose')
        self.taskMgrWidget.onDestroy()

class TaskManagerWidget(PandaObject):
    """
    TaskManagerWidget class: this class contains methods for creating
    a panel to control taskManager tasks.
    """

    def __init__(self, parent, taskMgr):
        """__init__(self)
        TaskManagerWidget class pops up a control panel to view/delete
        tasks managed by the taskManager.
        """
        # Make sure TK mainloop is running
        from TkGlobal import *
        # Record parent (used by ok cancel dialog boxes)
        self.parent = parent
        # Record taskManager
        self.taskMgr = taskMgr
        # Enable sending of spawn and remove messages
        self.taskMgr.setVerbose(1)
        # Init current task
        self.currentTask = None
        self.__taskDict = {}

        # Create widgets
        # Create a listbox
        self.taskListBox = Pmw.ScrolledListBox(
            parent,
            labelpos = NW, label_text = 'Tasks:',
            label_font=('MSSansSerif', 10, 'bold'),
            listbox_takefocus = 1,
            items = [],
            selectioncommand = self.setCurrentTask)
        self.taskListBox.pack(expand = 1, fill = BOTH)
                                           
        # Controls Frame
        controlsFrame = Frame(parent)
        self.removeButton = Button(controlsFrame, text = 'Remove Task',
                                   command = self.removeCurrentTask)
        #self.removeButton.pack(expand = 1, fill = X, side = LEFT)
        self.removeButton.grid(row = 0, col = 0, sticky = EW)
        self.removeMatchingButton = Button(controlsFrame,
                                           text = 'Remove Matching Tasks',
                                           command = self.removeMatchingTasks)
        #self.removeMatchingButton.pack(expand = 1, fill = X, side = LEFT)
        self.removeMatchingButton.grid(row = 0, col = 1, sticky = EW)

        self.taskMgrVerbose = IntVar()
        self.taskMgrVerbose.set(1)
        self.update = Button(
            controlsFrame,
            text = 'Update',
            command = self.updateTaskListBox)
        #self.update.pack(expand = 1, fill = X, side = LEFT)
        self.update.grid(row = 1, col = 0, sticky = EW)
        self.dynamicUpdate = Checkbutton(
            controlsFrame,
            text = 'Dynamic Update',
            variable = self.taskMgrVerbose,
            command = self.toggleTaskMgrVerbose)
        #self.dynamicUpdate.pack(expand = 1, fill = X, side = LEFT)
        self.dynamicUpdate.grid(row = 1, col = 1, sticky = EW)
        # Pack frames
        controlsFrame.pack(fill = X)
        controlsFrame.grid_columnconfigure(0, weight = 1)
        controlsFrame.grid_columnconfigure(1, weight = 1)
        
        # Add hook to spawnTaskEvents
        self.accept('TaskManager-spawnTask', self.spawnTaskHook)
        self.accept('TaskManager-removeTask', self.removeTaskHook)
        self.accept('TaskManager-setVerbose', self.updateTaskMgrVerbose)
        # Get listbox
        listbox = self.taskListBox.component('listbox')
        # Bind updates to arrow buttons
        listbox.bind('<KeyRelease-Up>', self.setCurrentTask)
        listbox.bind('<KeyRelease-Down>', self.setCurrentTask)
        # And grab focus (to allow keyboard navigation)
        listbox.focus_set()
        # Update listbox values
        self.updateTaskListBox()

    def setCurrentTask(self, event = None):
        index = int(self.taskListBox.curselection()[0])
        self.currentTask = self.__taskDict[index]

    def updateTaskListBox(self):
        # Get a list of task names
        taskNames = []
        self.__taskDict = {}
        count = 0
        for task in self.taskMgr.taskList:
            taskNames.append(task.name)
            self.__taskDict[count] = task
            count += 1
        self.taskListBox.setlist(taskNames)
        # And set current index (so keypresses will start with index 0)
        self.taskListBox.component('listbox').activate(0)
        # Select first item
        self.taskListBox.select_set(0)
        self.setCurrentTask()

    def toggleTaskMgrVerbose(self):
        taskMgr.setVerbose(self.taskMgrVerbose.get())
        if self.taskMgrVerbose.get():
            self.updateTaskListBox()

    def updateTaskMgrVerbose(self, value):
        self.taskMgrVerbose.set(value)

    def spawnTaskHook(self, task, name, index):
        self.updateTaskListBox()

    def removeTaskHook(self, task, name):
        self.updateTaskListBox()

    def removeCurrentTask(self):
        if self.currentTask:
            name = self.currentTask.name
            ok = 1
            if ((name == 'dataloop') or
                (name == 'tkloop') or
                (name == 'eventManager') or
                (name == 'igloop')):
                from tkMessageBox import askokcancel
                ok = askokcancel('TaskManagerControls',
                                 'Remove: %s?' % name,
                                 parent = self.parent,
                                 default = 'cancel')
            if ok:
                self.taskMgr.removeTask(self.currentTask)
                self.updateTaskListBox()

    def removeMatchingTasks(self):
        name = self.taskListBox.getcurselection()[0]
        ok = 1
        if ((name == 'dataloop') or
            (name == 'tkloop') or
            (name == 'eventManager') or
            (name == 'igloop')):
            from tkMessageBox import askokcancel
            ok = askokcancel('TaskManagerControls',
                             'Remove tasks named: %s?' % name,
                             parent = self.parent,
                             default = 'cancel')
        if ok:
            self.taskMgr.removeTasksNamed(name)
            self.updateTaskListBox()

    def onDestroy(self):
        self.ignore('TaskManager-spawnTask')
        self.ignore('TaskManager-removeTask')
        

