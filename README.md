# README #

ECMC Motion Control Module for EPICS

### What is this repository for? ###

* Motion Control with EtherCAT using Open Source
* Version
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)

### How do I get set up? ###

* Summary of set up
* Configuration
* Dependencies
* Database configuration
* How to run tests
* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines
* Please report issues here:
  https://ess-ics.atlassian.net/projects/ECMC/issues/

### Who do I talk to? ###

* Anders Sandström, Torsten Bögershausen
* anders.sandstrom@esss.se
* torsten.bogershausen@esss.se


## Release Notes ##

### ECMC 6.0.0 ###

Version 6.0.0 is not compeltely backward compatible since some changes 
in command-set have been made.

#### ECMC 6.0.0 ####
  * Remove ecmcAsynPortDriverAddParameter iocsh command
     Parameters are added automatically (when ethercat entries, axis objects, or .. are created/defined)

  * Add asynReport iocsh command:
    * asynReport 2: list all asyn parameters connected to records
    * asynReport 3: list all availabe parameters (that can be connected to records)
    NOTE: Also other asyn modules will display information when asynReport is executed.
          The motor record driver will output alot of information which could lead to that
          it's hard to find the information. Therefore ecmcReport command was added (which
          just print out information from ecmc).

  * Add ecmcReport iocsh command:
    Same as asynReport but only for ecmc (will not for instance list asynReport for motor record driver).

  * Add ONE, ZERO entries for each slave withnread and write access.
    This to allow more flexibilty when linking values:
     ec<m-id>.s<s-id>.ONE  : 0xFFFFFFFF (a 32 bit register filled with 1)
     ec<m-id>.s<s-id>.ZERO : 0x0 (a 32 bit register filled with 0)    
     example: ec0.s23.ONE can be used.
     NOTE: The "ONE" and "ZERO" entries for slave -1 is still available for backward compatibility
     (ec<m-id>.s-1.ONE/ZERO)

  * Add status word for objects as asyn parameters:
      * Ec           :  ec<m-id>.masterstatus
      * EcSlave      :  ec<m-id>.s<s-id>.slavestatus : 
             * bit 0      : online
             * bit 1      : operational
             * bit 2      : alstate.init
             * bit 3      : alstate.preop
             * bit 4      : alstate.safeop
             * bit5       : alstate.op
             * bit 16..32 : entrycounter
      - Axis         :  ax<axis id>.status
      - Data Storage : ds<id>.status

  - Add support for modulo movement (both jog/constant velo and positioning):
    - Cfg.SetAxisModRange(<axis id>, <mod range>)":
      <mod range>: Encoder and Trajectory range will be within 0..<mod range>
    - Cfg.SetAxisModType(<axis id>, <mod type>)" For positioning
      <mod type>:   0 = Normal
                    1 = Always Fwd
                    2 = Always Bwd
                    3 = Closets way (minimize travel)
        
  - Add time stamp in ECMC layer:
    Time stamp source can be choosen by setting record TSE-field
     TSE =  0 EPCIS timestamp
     TSE = -2 ECMC timestamp, 
  
  - Update sample rates from skip cycles to rate in ms

  - Remove command: Cfg.SetAxisGearRatio(). Obsolete, handled through axis plc instaed.

  - ecmcConfigOrDie: Add printout of command if fail (error)

  - Add asyn parameter that updates when all other parameters have been updated:
      ecmc.updated of type asynInt32ArrayIn

  - Update to new asynPortDriver constructor (to remove warning)

  - Add command Cfg.EcSlaveVerify(). Check that a slave at a certain bus-position have the correct vendor-id and product-id.
    This command is added to all hardware snippets in ecmctraining to ensure that the slave is of correct type.
  
  - Add range checks for ec-entry writes (based on bit length and signed vs unsigned)

  - Add memory lock (memlockall) and allocation of stack memory (good for rt performance)

  PLCs: 
  - Replace axis command transform with axis dedcicated PLC object (executed in sync before axis object), 
    in order to use same syntax in entire ECMC.
    Many new variables and functions can now be used in the axis sync PLC (same as normal PLC)
    Currentlly the following indexes apply:
     1. normal PLCs are in range 0..ECMC_MAX_PLCS-1
     2. Axis PLCs are in range ECMC_MAX_PLCS..ECMC_MAX_AXES-1 

    PLCs can set:
     1. the current setpoint of an axis by writing to: ax<id>.traj.setpos. The axis will use this setpoint if trajectory source is set to external
     2. the cutrrent encoder actual position by writing to: ax<id>.enc.actpos. The axis will use this setpoint if encoder source is set to external
     All other PLC variables can be used in the axis plcs (see file plcSyntax.plc in ecmctraining)

    Rename commands:
     - Cfg.GetAxisEnableCommandsTransform()     ->  Cfg.GetAxisPLCEnable()          
       Check if Axis PLC is enabled.

     - Cfg.SetAxisEnableCommandsTransform()     ->  Cfg.SetAxisPLCEnable()
       Enable axis PLC (the plc will execute only if enabled

     - Cfg.GetAxisTransformCommandExpr()        ->  Cfg.GetAxisPLCExpr()  
       Read current PLC code for axis PLC.

     - Cfg.SetAxisTransformCommandExpr()        ->  Cfg.SetAxisPLCExpr()   
       Write PLC code for axis PLC.

     - Cfg.GetAxisEnableCommandsFromOtherAxis() ->  Cfg.GetAxisAllowCommandsFromPLC()
       Check if axis allows commands/writes from any PLC.

     - Cfg.SetAxisEnableCommandsFromOtherAxis() ->  Cfg.SetAxisAllowCommandsFromPLC()
       Allow commands (writes) from any PLC object (includig axis PLC)

     - Cfg.SetAxisEncExtVelFilterEnable()   -> Cfg.SetAxisPLCEncVelFilterEnable()

     - Cfg.SetAxisTrajExtVelFilterEnable()   -> Cfg.SetAxisPLCTrajVelFilterEnable()

    Add Commands:
     - Cfg.AppendAxisPLCExpr(<axis id>, <code string>)
       Append a line of code to a Axis PLC
     - Cfg.LoadAxisPLCExpr(<axis id>, <filename>) 
       Load a file with code to a Axis PLC

   - Remove unused files (ecmcMasterSlave*, ecmcCommandTransform*)

  - Add plc functions for average, min and max of a data storage:
     - ds_get_avg(<ds id>) : Get average (usefull as lowpass filter)
     - ds_get_min(<ds id>) : Get minimum
     - ds_get_max(<ds id>) : Get maximum

     All three functions only use the data that has been written to the storage (not the empty/free space). 
  
  - Add interlock variables in PLC : 
    - axis<id>.mon.ilockbwd : allow motion in backward dir (stop if set to 0) 
    - axis<id>.mon.ilockfwd : allow motion in forward dir (stop if set to 0)

  - Add licence file
  
  - Add basic implemetation of a basic comminsioning GUI (ecmccomgui). Currentlly in different repo:
    based on pyQT, pyEpics

    -Add command that verifies a SDO enries value:
                 Cfg.EcVerifySdo(uint16_t slave_position,
                                 uint16_t sdo_index,
                                 uint8_t sdo_subindex,
                                 uint32_t verValue,
                                 int byteSize)

    -Add extra command for create an axis
                 Cfg.CreateAxis(int axisIndex,
                                int axisType,  //Optional, defaults to normal axis
                                int drvType)   //optional, defaults to stepper axis 
    
    - Remove Command Cfg.SetAxisDrvType(). Use Cfg.CreateAxis(<id>,<type>,<drvType>) instead

    - Add command for remote access to iocsh over asynRecord: Cfg.IocshCmd=<cmd>

    - Add command to set velocity low pass filter size for encoder (actpos) and trajetory (setpos):
      Needed if resolution of encoder is low. Since the velocity is used for feed forward purpose. 
      Two cases:
      1. For internal source: 
        Cfg.SetAxisEncVelFilterSize(<axisId>,<filtersize>)"
      2. External velocity  (values from PLC code). 
       Cfg.SetAxisPLCEncVelFilterSize(<axisId>,<filtersize>)"
       Cfg.SetAxisPLCTrajVelFilterSize(<axisId>,<filtersize>)"
           
## Release Notes ##
    * Remove manual motion mode (not needed.. motors can be run manually directlly from ethercat entries)
    * Add PID controller in PLC lib
    * Add licence headers to all source files
    * Consider change error handling system to exception based (try catch)
    * Clean up in axis sub objects:
           * emcmMonitor:  Add verify setpoint function (mostly rename)
           * ecmcTrajectory: Make more independent of other code
           * ecmcTrajectory: Make Spline Version
    * Implementation of robot and hexapod kinematics? How todo best possible?
    * Make possible to add ethercat hardware on the fly (hard, seems EPICS do not support dynamic load of records)
           * Only stop motion when the slaves used by axis are in error state or not reachable.
    * Redundacy test




