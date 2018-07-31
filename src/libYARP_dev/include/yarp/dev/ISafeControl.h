/*
 * Copyright (C) 2017 ICub Facility - Istituto italiano di Tecnologia
 * Author: Alessandro Scalzo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#ifndef YARP_DEV_ISAFECONTROL_H
#define YARP_DEV_ISAFECONTROL_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/sig/Vector.h>

namespace yarp
{
    namespace dev
    {
        /**
         * @ingroup dev_iface_other
         *
         * A generic safe control interface
         */
        class YARP_dev_API ISafeControl
        {
        public:
            ISafeControl(){};
            //virtual ~ISafeControl(){};
    
            /**
             * Check next configuration
             * @param position joint configuration
             * @return true/false.
             */
            virtual bool checkPosition(double *position, int *jointMap, int nJoints) = 0;

            /**
             * Check velocity at present configuration
             * @param velocity joint velocity
             * @return true/false.
             */
            virtual bool checkVelocity(double *velocity, int *jointMap, int nJoints) = 0;
        };
    }
}

#endif // YARP_DEV_ISAFECONTROL_H
