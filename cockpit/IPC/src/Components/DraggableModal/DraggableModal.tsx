/* eslint-disable react/jsx-props-no-spreading */
/* eslint-disable react/function-component-definition */
import { Title } from '@patternfly/react-core';
import React from 'react';
import Draggable, { DraggableData, DraggableEvent } from 'react-draggable';

interface DraggableProps {
  iface: any,
  children: any,
  isOpen: boolean,
  [key: string]: any; // for other props
}

type Position = {
  xRate: number;
  yRate: number;
};

const DraggableModal: React.FC<DraggableProps> = ({
  iface, children, isOpen, ...rest
}) => {
  const [currentPosition, setCurrentPosition] = React.useState<Position>({
    xRate: 20,
    yRate: 20,
  });

  const onDrag = (e: DraggableEvent, data: DraggableData) => {
    setCurrentPosition({ xRate: data.lastX, yRate: data.lastY });
  };

  return (
    <Draggable
      position={{
        x: currentPosition.xRate,
        y: currentPosition.yRate,
      }}
      onDrag={onDrag}
      // handle=".modal-header"
      key={iface.InterfaceName}
    >
      <div style={{
        visibility: isOpen ? 'visible' : 'hidden',
        backgroundColor: 'rgb(18, 18, 18,0.9)',
        width: '500px',
        height: '500px',
        position: 'absolute',
        zIndex: 100,
        borderRadius: '0.6rem',
      }}
      >
        <Title headingLevel="h1" size="lg" style={{ color: 'white' }}>
          {iface.interfaceName}
        </Title>
        <div aria-label={`Watcher-${iface.interfaceName}`} {...rest} style={{ overflowY: 'scroll', maxHeight: 'calc(100% - 2rem)' }}>
          {children}
        </div>
      </div>
    </Draggable>
  );
};

export default DraggableModal;
