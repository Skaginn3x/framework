/* eslint-disable no-param-reassign */
/* eslint-disable max-len */
import React, { useEffect, useRef, useState } from 'react';
import { Title } from '@patternfly/react-core';
import { loadExternalScript } from 'src/Components/Interface/ScriptLoader';
import { DarkModeType } from 'src/App';
import parse from 'html-react-parser';
import { TFC_DBUS_DOMAIN, TFC_DBUS_ORGANIZATION } from 'src/variables';

declare global {
  interface Window { cockpit: any; }
}

// eslint-disable-next-line react/function-component-definition
const VisualState:React.FC<DarkModeType> = ({ isDark }) => {
  const [svg, setSVG] = useState<any>();
  const [processDBUS, setProcessDBUS] = useState<any>();
  const [data, setData] = useState<any>({});
  const dataRef = useRef(data);
  const initialsvg = `<?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <svg
       sodipodi:docname="demo.svg"
       inkscape:version="1.3 (0e150ed6c4, 2023-07-21)"
       viewBox="0 0 1062.9921 708.66141"
       version="1.1"
       height="200mm"
       width="350mm"
       id="svg2"
       xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
       xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
       xmlns="http://www.w3.org/2000/svg"
       xmlns:svg="http://www.w3.org/2000/svg"
       xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
       xmlns:cc="http://creativecommons.org/ns#"
       xmlns:dc="http://purl.org/dc/elements/1.1/">
       xmlns:dbus="http://www.skaginn3x.com/"
       xmlns:tfc="http://www.skaginn3x.com/"
      <defs
         id="defs4">
        <rect
           x="942.14949"
           y="596.79613"
           width="112.81199"
           height="30.664053"
           id="rect2" />
        <rect
           x="942.14948"
           y="596.79614"
           width="112.81199"
           height="30.664053"
           id="rect2-6" />
      </defs>
      <sodipodi:namedview
         inkscape:document-rotation="0"
         inkscape:zoom="1.2705736"
         inkscape:window-y="0"
         inkscape:window-x="0"
         inkscape:window-width="1916"
         inkscape:window-maximized="1"
         inkscape:window-height="1166"
         inkscape:snap-global="false"
         inkscape:pageshadow="2"
         inkscape:pageopacity="0.0"
         inkscape:document-units="px"
         inkscape:cy="386.43962"
         inkscape:cx="323.86946"
         inkscape:current-layer="g74"
         showgrid="false"
         pagecolor="#ffffff"
         fit-margin-top="5"
         fit-margin-right="5"
         fit-margin-left="5"
         fit-margin-bottom="5"
         borderopacity="1.0"
         bordercolor="#666666"
         id="base"
         inkscape:pagecheckerboard="0"
         inkscape:showpageshadow="0"
         inkscape:deskcolor="#505050" />
      <metadata
         id="metadata7">
        <rdf:RDF>
          <cc:Work
             rdf:about="">
            <dc:format>&#10;                    image/svg+xml&#10;                </dc:format>
            <dc:type
               rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
          </cc:Work>
        </rdf:RDF>
      </metadata>
      <g
         inkscape:label="Layer 1"
         inkscape:groupmode="layer"
         transform="translate(-100.80896,695.09779)"
         id="layer1" />
      <g
         transform="translate(-617.8865,588.44598)"
         inkscape:label="Layer 2"
         id="g74">
        <text
           id="text879"
           y="-199.26302"
           x="1043.2826"
           style="font-style:normal;font-weight:normal;font-size:37.5px;line-height:1.25;font-family:sans-serif;fill:#000000;fill-opacity:1;stroke:none;stroke-width:0.9375"
           xml:space="preserve"><tspan
             style="stroke-width:0.9375"
             y="-199.26302"
             x="1043.2826"
             id="tspan877"
             sodipodi:role="line" /></text>
        <rect
           style="fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:0.938976;stroke-dasharray:none;stroke-opacity:1"
           id="rect1"
           width="117.75139"
           height="100.42908"
           x="1277.8844"
           y="-61.452744" />
        <rect
           style="fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:0.938976;stroke-dasharray:none;stroke-opacity:1"
           id="rect1-5"
           width="117.75139"
           height="100.42908"
           x="934.40692"
           y="-61.975235" />
        <rect
           style="fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:1.24539;stroke-dasharray:none;stroke-opacity:1"
           id="rect1-3"
           width="117.44498"
           height="177.13058"
           x="1278.6816"
           y="-248.03934" />
        <rect
           style="fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:1.24539;stroke-dasharray:none;stroke-opacity:1"
           id="rect1-3-6"
           width="117.44498"
           height="177.13058"
           x="1279.601"
           y="-435.01361" />
        <rect
           style="fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:1.77909;stroke-dasharray:none;stroke-opacity:1"
           id="rect1-3-6-3"
           width="116.91128"
           height="363.12451"
           x="935.49683"
           y="-435.05713" />
        <rect
           style="fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:1.24539;stroke-dasharray:none;stroke-opacity:1"
           id="rect1-3-6-7"
           width="117.44498"
           height="177.13058"
           x="-435.41144"
           y="1075.4885"
           transform="matrix(0,1,1,0,0,0)" />
        <ellipse
           style="fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:0.669685;stroke-dasharray:none;stroke-opacity:1"
           id="path1"
           cx="1421.8469"
           cy="-43.435825"
           rx="8.4046373"
           ry="8.0472383"
           tfcSignal="com.skaginn3x.operations.def.bool.run_button"
           tfcOn_true="green" 
           tfcOn_false="red"/>
        <text
           xml:space="preserve"
           transform="matrix(0.93749998,0,0,0.93749998,529.30384,-588.44598)"
           id="text1"
           style="white-space:pre;shape-inside:url(#rect2);fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:0.714331;stroke-dasharray:none;stroke-opacity:1"
           dbusInterface="com.skaginn3x.Operations"
           dbusPath="/com/skaginn3x/StateMachines"
           dbusService="com.skaginn3x.tfc.operations.def"><tspan
             x="942.15039"
             y="607.71538"
             id="tspan3">\${State}</tspan></text>
        <text
           xml:space="preserve"
           transform="matrix(0.93749998,0,0,0.93749998,431.60097,-577.8504)"
           id="text1-2"
           style="white-space:pre;shape-inside:url(#rect2-6);fill:#000000;fill-opacity:0;stroke:#000000;stroke-width:0.714331;stroke-dasharray:none;stroke-opacity:1"
           tfcSignal="com.skaginn3x.sensor_control.sadd.json.discharge_request"><tspan
             x="942.15039"
             y="607.71538"
             id="tspan4">\${} Hz</tspan></text>
      </g>
    </svg>` as any;

  function toggleDarkMode(svgString: string) {
    if (isDark) {
      svgString = svgString.replaceAll('stroke:#000000', 'stroke:#EEE');
      svgString = svgString.replaceAll('fill:#000000', 'fill:#EEE');
    } else {
      svgString = svgString.replaceAll('stroke:#EEE', 'stroke:#000000');
      svgString = svgString.replaceAll('fill:#EEE', 'fill:#000000');
    }
    return svgString;
  }

  function parseDBusTextObjects(svgString: string) {
    const parser = new DOMParser();
    const xmlDoc = parser.parseFromString(svgString, 'text/xml');
    // Check for parsing errors
    if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
      console.error('Error parsing XML');
      return [];
    }

    const dbusElements = xmlDoc.querySelectorAll('[dbusInterface], [dbusPath], [dbusService]');

    const dbusObjects = Array.from(dbusElements).map((element) => {
      // Extract the placeholder value inside ${}
      const placeholderMatch = element.textContent?.match(/\$\{([^}]+)\}/);
      const placeholder = placeholderMatch ? placeholderMatch[1] : '';

      return {
        interface: element.getAttribute('dbusInterface') ?? '',
        path: element.getAttribute('dbusPath') ?? '',
        service: element.getAttribute('dbusService') ?? '',
        property: placeholder,
      };
    });

    return dbusObjects;
  }

  function replaceDBusSignals(svgString: string): string {
    // data is of format: [{ com.skaginn3x.operations.def.bool.run_button: true }, { com.skaginn3x.operations.def.bool.run_button: false }}]
    const parser = new DOMParser();
    const xmlDoc = parser.parseFromString(svgString, 'text/xml');
    // Check for parsing errors
    if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
      console.error('Error parsing XML');
      return '';
    }

    const dbusElements = xmlDoc.querySelectorAll('[tfcSignal]');

    dbusElements.forEach((element) => {
      // Check if the element matches the specified criteria
      const signal = element.getAttribute('tfcSignal');
      console.log('signal', signal);
      console.log('1');
      if (element.hasAttribute('tfcOn_true') && element.hasAttribute('tfcOn_false')) {
        const onTrue = element.getAttribute('tfcOn_true');
        const onFalse = element.getAttribute('tfcOn_false');
        console.log('2');
        // Replace placeholder in the text content
        if (signal && onTrue && onFalse) {
          element.setAttribute('style', '');
          console.log('3', element.getAttribute('style'));
          if (data[signal] === true) {
            element.setAttribute('fill', onTrue);
            element.setAttribute('stroke', onTrue);
          } else {
            element.setAttribute('fill', onFalse);
            element.setAttribute('stroke', onFalse);
          }
        }
      } else if (signal) {
        console.log('4');
        // Replace placeholder in the text content
        const tspan = element.querySelector('tspan');
        // replace
        if (tspan && tspan.textContent?.includes('${}')) {
          tspan.textContent = tspan.textContent.replace('${}', data[signal]);
        }
      }
    });
    // serialize the XML to a string
    return new XMLSerializer().serializeToString(xmlDoc);
  }

  function parseDBusSignals(svgString: string) {
    // Returns names of signals
    const parser = new DOMParser();
    const xmlDoc = parser.parseFromString(svgString, 'text/xml');
    // Check for parsing errors
    if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
      console.error('Error parsing XML');
      return [];
    }

    const dbusElements = xmlDoc.querySelectorAll('[tfcSignal]');
    const dbusSignals = Array.from(dbusElements).map((element) => {
      // Extract the placeholder value inside ${}
      const signal = element.getAttribute('tfcSignal') ?? '';
      return signal;
    });

    return dbusSignals.reduce((acc: any, cur: any) => {
      acc[cur] = null;
      return acc;
    }, {});
  }

  function replaceSvgPlaceholders(svgString: string, interfaceName: string, process: string, path: string, placeholder:string, replacementValue: string) {
    const parser = new DOMParser();
    const xmlDoc = parser.parseFromString(svgString, 'text/xml');
    // Check for parsing errors
    if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
      console.error('Error parsing XML');
      return '';
    }

    // Find all text elements that might contain placeholders
    const textElements = xmlDoc.querySelectorAll('text');

    textElements.forEach((element) => {
      // Check if the element matches the specified criteria
      if (element.getAttribute('dbusInterface') === interfaceName
          && element.getAttribute('dbusPath') === path
          && element.getAttribute('dbusService') === process) {
        // Replace placeholder in the text content
        const tspan = element.querySelector('tspan');
        if (tspan && tspan.textContent?.includes(`\${${placeholder}}`)) {
          tspan.textContent = tspan.textContent.replace(`\${${placeholder}}`, replacementValue);
        }
      }
      if (element.getAttribute('style')?.includes('stroke-opacity') && element.getAttribute('style')?.includes('fill-opacity')) {
        // Replace placeholder in the text content
        const style = element.getAttribute('style');
        if (style) {
          let newStyle = element.getAttribute('style');
          newStyle = newStyle!.replace('stroke-opacity:1', 'stroke-opacity:0');
          newStyle = newStyle.replace('fill-opacity:0', 'fill-opacity:1');
          element.setAttribute('style', newStyle);
        }
      }
    });
    // serialize the XML to a string
    return new XMLSerializer().serializeToString(xmlDoc);
  }

  function handleChange() {
    const interfacesToChange = parseDBusTextObjects(initialsvg);
    const proxies = interfacesToChange.map((interfaceToChange) => {
      if (window.cockpit) {
        const dbus = window.cockpit.dbus(interfaceToChange.service);
        return {
          interface: interfaceToChange.interface,
          path: interfaceToChange.path,
          process: interfaceToChange.service,
          placeholder: interfaceToChange.property,
          proxy: dbus.proxy(interfaceToChange.interface, interfaceToChange.path),
        };
      }
      return {};
    });
    let newSvg = replaceDBusSignals(initialsvg);
    // resolve promises
    Promise.all(proxies.map(async (iface: any) => {
      await iface.proxy.wait();
      const replacementValue = iface.proxy.data[iface.placeholder];
      newSvg = replaceSvgPlaceholders(newSvg, iface.interface, iface.process, iface.path, iface.placeholder, replacementValue);
    })).then(() => {
      setSVG(toggleDarkMode(newSvg));
    });
  }

  useEffect(() => {
    loadExternalScript(() => {
      setData(parseDBusSignals(initialsvg));
      setProcessDBUS(window.cockpit.dbus(null));
      handleChange();
    });
  }, []);

  useEffect(() => {
    if (!processDBUS) return;

    const match = {
      path_namespace: `/${TFC_DBUS_DOMAIN}/${TFC_DBUS_ORGANIZATION}`,
    };
    processDBUS.watch(match);

    const notifyHandler = (newData: any) => {
      const removePath = newData.detail[Object.keys(newData.detail)[0]];
      console.log('removePath', removePath);
      console.log('dataref', dataRef.current);
      const ifaceName = Object.keys(removePath)[0];
      if (Object.keys(dataRef.current).includes(ifaceName)) {
        setData((prevState: any) => {
          const newState = { ...prevState };
          console.log('newState', newState);
          newState[ifaceName] = removePath[ifaceName].Value;
          return newState;
        });
      }
    };

    processDBUS.addEventListener('notify', notifyHandler);
    // eslint-disable-next-line consistent-return
    return () => {
      processDBUS.removeEventListener('notify', notifyHandler);
    };
  }, [processDBUS]);

  useEffect(() => {
    dataRef.current = data;
    handleChange();
  }, [data]);

  useEffect(() => {
    if (svg) {
      setSVG(toggleDarkMode(svg));
    }
  }, [isDark]);

  return (
    <div style={{ color: isDark ? '#EEE' : '#111' }}>
      <Title headingLevel="h1" size="2xl">Visual States</Title>
      <br />
      {svg && parse(svg)}
    </div>
  );
};

export default VisualState;
