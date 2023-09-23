/* eslint-disable react/function-component-definition */
import React from 'react';

interface CircleProps {
  size: string;
  color: string;
}

export const Circle: React.FC<CircleProps> = ({ size, color }) => (
  <div
    style={{
      height: size,
      width: size,
      borderRadius: `calc(${size} / 2)`,
      backgroundColor: color,
    }}
  />
);

export default Circle;
