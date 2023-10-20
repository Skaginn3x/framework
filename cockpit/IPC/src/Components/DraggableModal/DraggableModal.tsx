/* eslint-disable jsx-a11y/no-noninteractive-tabindex */
/* eslint-disable jsx-a11y/no-static-element-interactions */
/* eslint-disable react/jsx-props-no-spreading */
/* eslint-disable react/function-component-definition */
import { Title } from '@patternfly/react-core';
import React from 'react';
import Draggable, { DraggableData, DraggableEvent } from 'react-draggable';
import { TimesIcon } from '@patternfly/react-icons';
import { removeSlotOrg } from '../Form/WidgetFunctions';

interface DraggableProps {
  iface: any,
  children: any,
  isOpen: boolean,
  visibilityIndex: number,
  onClose: () => void,
  datapoints: any[],
  [key: string]: any; // for other props
}

type Position = {
  xRate: number;
  yRate: number;
};

const DraggableModal: React.FC<DraggableProps> = ({
  iface, children, isOpen, visibilityIndex, onClose, datapoints, ...rest
}) => {
  const [currentPosition, setCurrentPosition] = React.useState<Position>({
    xRate: 0,
    yRate: 0,
  });
  const scrollRef = React.useRef<HTMLDivElement | null>(null);

  React.useEffect(() => {
    if (scrollRef.current) {
      const element = scrollRef.current;
      const isNearBottom = element.scrollTop + element.clientHeight >= element.scrollHeight - 50; // '50' is a threshold, you can adjust it.

      if (isNearBottom) {
        element.scrollTop = element.scrollHeight;
      }
    }
  }, [children]);

  const onDrag = (e: DraggableEvent, data: DraggableData) => {
    setCurrentPosition({ xRate: data.x, yRate: data.y });
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if ((e.ctrlKey || e.metaKey) && e.key === 'a') {
      e.preventDefault();

      const selection = window.getSelection();
      const range = document.createRange();
      if (!selection || !range) return;
      range.selectNodeContents(e.currentTarget); // select contents of the current div
      selection.removeAllRanges();
      selection.addRange(range);
    }
  };

  const handleCopy = (e: React.ClipboardEvent) => {
    e.preventDefault(); // Prevent default copy
    const copyString = datapoints.map((datapoint) => {
      const formatted = `'${new Date(datapoint.timestamp).toISOString()}.${new Date(datapoint.timestamp).getMilliseconds()}'`;
      return `${formatted}, ${datapoint.value}`;
    }).join('\n');

    e.clipboardData.setData('text/plain', copyString); // Set the clipboard data
  };

  return (
    <Draggable
      position={{
        x: currentPosition.xRate,
        y: currentPosition.yRate,
      }}
      onDrag={onDrag}
      handle=".handle"
      key={iface.InterfaceName}
      bounds="parent"
    >

      <div
        hidden={!isOpen}
        style={{
          backgroundColor: 'rgb(18, 18, 18,0.9)',
          width: '440px',
          height: '350px',
          position: 'absolute',
          top: `${75 + 10 * visibilityIndex}px`,
          left: `${20 + 10 * visibilityIndex}px`,
          zIndex: 100,
          borderRadius: '0.6rem',
          border: '2px solid #333',
        }}
      >
        <div
          className="handle"
          style={{
            display: 'flex', flexDirection: 'row', alignContent: 'center', justifyContent: 'space-between',
          }}

        >
          <div style={{ width: '16px', height: '16px', margin: '0.5rem' }} />
          <Title headingLevel="h1" size="md" style={{ color: 'white', marginTop: '0.4rem' }}>
            {removeSlotOrg(iface.interfaceName)}
          </Title>
          <TimesIcon onClick={() => onClose()} color="#EEE" style={{ margin: '0.5rem' }} />
        </div>
        <div
          aria-label={`Watcher-${iface.interfaceName}`}
          style={{ overflowY: 'scroll', height: 'calc(100% - 3rem)' }}
          ref={scrollRef}
          onKeyDown={handleKeyDown}
          tabIndex={0}
          onCopy={handleCopy}
          {...rest}
        >
          {children}
        </div>
      </div>
    </Draggable>
  );
};

export default DraggableModal;
