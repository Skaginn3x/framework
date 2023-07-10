/* eslint-disable @typescript-eslint/no-use-before-define */
import React, { useEffect, useState } from 'react';
import { Button, Title } from '@patternfly/react-core';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'bootstrap-icons/font/bootstrap-icons.css';
import './Configurator.css';
import validator from '@rjsf/validator-ajv8';
import Form from '@rjsf/core';
import Collapse from '@mui/material/Collapse';
import ArrowDropDownIcon from '@mui/icons-material/ArrowDropDown';
import ArrowRightIcon from '@mui/icons-material/ArrowRight';
import {
  demoRJSFSchema, demoRJSFData2, demoRJSFSchema2, demoUiSchema,
} from './demoData';
import Generator from '../Components/Form/Form';

declare global {
  interface Window { cockpit: any; }
}

function ArrayFieldTemplate(
  props: {
    className?: string | undefined; items: any[];
    canAdd?: any;
    onAddClick: React.MouseEventHandler<HTMLButtonElement> | undefined;
  },
) {
  const {
    items, className, canAdd, onAddClick,
  } = props;
  const [expanded, setExpanded] = useState(items.map(() => false));

  const handleExpandClick = (index: number) => {
    setExpanded((prevExpanded) => {
      const newExpanded = [...prevExpanded];
      newExpanded[index] = !newExpanded[index];
      return newExpanded;
    });
  };

  // Add a new collapsed item when onAddClick is triggered
  const handleAddClick = (event: React.MouseEvent<HTMLButtonElement, MouseEvent>) => {
    setExpanded((prevExpanded) => [...prevExpanded, true]);
    if (onAddClick) { onAddClick(event); }
  };

  return (
    <div className={className}>
      {items && items.map((element, index) => (
        <div key={element.key} className={element.className} style={{ marginBottom: '1rem' }}>
          <button
            style={{
              all: 'unset',
              width: '100%',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              marginBottom: '0rem',
              backgroundColor: '#f4f4f4',
              borderRadius: expanded[index] ? '0.5rem 0.5rem 0rem 0rem' : '0.5rem',
            }}
            onClick={() => handleExpandClick(index)}
            type="button"
          >
            <div style={{ width: '40px' }}> </div>
            <p style={{ fontSize: '1.1rem', margin: '0rem' }}>{`Item ${index + 1}`}</p>
            {expanded[index]
              ? <ArrowDropDownIcon style={{ height: '2.3rem', width: '2.3rem' }} />
              : <ArrowRightIcon style={{ height: '2.3rem', width: '2.3rem' }} />}
          </button>
          <Collapse in={expanded[index]}>
            <div style={{
              display: 'flex',
              flexDirection: 'column',
              backgroundColor: '#FAFAFA',
              alignItems: 'center',
              borderRadius: '0rem 0rem 0.5rem 0.5rem',
            }}
            >
              <div style={{ width: '90%' }}>{element.children}</div>
              <Button
                type="button"
                variant="danger"
                onClick={element.onDropIndexClick(element.index)}
                style={{ width: '5rem', alignSelf: 'end', margin: '0.6rem' }}
              >
                Delete
              </Button>
            </div>
          </Collapse>
        </div>
      ))}

      {canAdd && (
        <div className="row">
          <p className="col-xs-3 col-xs-offset-9 array-item-add text-right">
            <Button onClick={handleAddClick} type="button" variant="primary">
              Add Item
            </Button>
          </p>
        </div>
      )}
    </div>
  );
}

ArrayFieldTemplate.defaultProps = {
  className: '',
  canAdd: false,
};

function simplifySchemaTypes(schema: any) {
  if (Array.isArray(schema.type) && schema.type.length === 1) {
    // eslint-disable-next-line prefer-destructuring, no-param-reassign
    schema.type = schema.type[0];
  }

  if (schema.properties) {
    // eslint-disable-next-line no-restricted-syntax, guard-for-in
    for (const key in schema.properties) {
      simplifySchemaTypes(schema.properties[key]);
    }
  }

  if (schema.$defs) {
    // eslint-disable-next-line no-restricted-syntax, guard-for-in
    for (const key in schema.$defs) {
      simplifySchemaTypes(schema.$defs[key]);
    }
  }

  return schema;
}

export default function Configurator() {
  const demoData = [
    {
      direction: 'as_is',
      in_or_out: 'std::monostate',
    },
    {
      direction: 'as_is',
      in_or_out: 'std::monostate',
    },
    {
      direction: 'as_is',
      in_or_out: 'std::monostate',
    },
    {
      direction: 'as_is',
      in_or_out: 'std::monostate',
    },
    {
      direction: 'as_is',
      in_or_out: 'std::monostate',
    },
    {
      direction: 'as_is',
      in_or_out: 'std::monostate',
    },
  ];

  const demoRJSF = demoRJSFSchema();
  const demo2RJSF = simplifySchemaTypes(demoRJSFSchema2());
  const demo2RJSFData = demoRJSFData2();
  console.log('demo2RJSFData: ', demo2RJSFData);
  console.log('demo2RJSF: ', demo2RJSF);

  const [names, setNames] = useState<string[]>([]);
  const [formData, setFormData] = useState<any>({ demo: { config: demoData } });
  const [schemas, setSchemas] = useState<any>({ demo: demoRJSF });

  // Load cockpit.js and get dbus names
  useEffect(() => {
    const externalScript = '../base1/cockpit.js';
    let script = document.querySelector(`script[src="${externalScript}"]`) as HTMLScriptElement;

    const handleScriptLoad = () => {
      const dbus = window.cockpit.dbus('org.freedesktop.DBus');
      const proxy = dbus.proxy();
      proxy.wait().then(() => {
        proxy.call('ListNames').then((Allnames: any[]) => {
          // if name includes skaginn3x, get interfaces
          console.log('names: ', Allnames[0]);
          setNames(Allnames[0].filter((name: string) => name.includes('config') && !name.includes('ipc_ruler')));
        });
      });
    };

    if (!script) {
      script = document.createElement('script');
      script.src = externalScript;
      script.async = true;
      script.addEventListener('load', handleScriptLoad);
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
      document.body.appendChild(script);
    } else {
      script.addEventListener('load', handleScriptLoad);
      script.addEventListener('error', (e) => { console.error('Error loading script', e); });
    }
  }, []);

  // Get data and schema for each name and store in states
  useEffect(() => {
    return;
    if (names.length > 0) {
      names.forEach((name: string) => {
        const dbus = window.cockpit.dbus(name);
        const OBJproxy = dbus.proxy(name);
        OBJproxy.wait().then(() => {
          const { data } = OBJproxy;
          const parsedData = JSON.parse(data.config[0].replace('\\"', '"'));
          const parsedSchema = JSON.parse(data.config[1].replace('\\"', '"'));
          console.log('parsedSchema: ', parsedSchema);
          console.log('parsedData: ', parsedData);
          // eslint-disable-next-line arrow-body-style
          setSchemas((prevState: any) => {
            return {
              ...prevState,
              [name]: parsedSchema,
            };
          });
          // eslint-disable-next-line arrow-body-style
          setFormData((prevState: any) => {
            return {
              ...prevState,
              [name]: { config: parsedData },
            };
          });
        });
      });
    }
  }, [names]);

  const updateFormData = (name: string, data: any) => {
    if (!data) return;
    setFormData((prevState: any) => ({
      ...prevState,
      [name]: data,
    }));
  };

  useEffect(() => {
    console.log('formData: ', formData);
  }, [formData]);

  return (
    <div style={{ minWidth: '500px', maxWidth: '40vw' }}>
      <Title headingLevel="h1" size="xl" style={{ marginBottom: '1rem' }}> Configurator - Time For Change</Title>
      {names.length && Object.keys(schemas).length && Object.keys(schemas).map((name: string) => {
        if (schemas[name] && formData[name]) {
          return (
            <>
              <Title headingLevel="h2" size="xl" style={{ marginBottom: '1rem' }}>{name || 'Unknown name'}</Title>
              <Form
                schema={schemas[name]}
                formData={formData[name]}
                templates={{ ArrayFieldTemplate }}
                validator={validator}
                onChange={(e) => updateFormData(name, e.formData)}
                onError={() => console.log('errors')}
              >
                { /* eslint-disable-next-line react/jsx-no-useless-fragment */}
                <></>
              </Form>
              <div style={{ marginBottom: '2rem' }} />
            </>
          );
        }
        return null;
      })}
      <Generator inputSchema={demoUiSchema()} />
    </div>
  );
}
