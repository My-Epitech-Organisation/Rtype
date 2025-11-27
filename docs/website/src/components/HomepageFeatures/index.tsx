import type {ReactNode} from 'react';
import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';

type FeatureItem = {
  title: string;
  emoji: string;
  description: ReactNode;
};

const FeatureList: FeatureItem[] = [
  {
    title: 'Entity Component System',
    emoji: '🎮',
    description: (
      <>
        Built with a modular ECS architecture that separates data from behavior,
        allowing for flexible and efficient game object management.
      </>
    ),
  },
  {
    title: 'Network Multiplayer',
    emoji: '🌐',
    description: (
      <>
        UDP-based networking with client-server architecture. Support for
        multiple players in real-time space shooter gameplay.
      </>
    ),
  },
  {
    title: 'Modern C++20',
    emoji: '⚡',
    description: (
      <>
        Leverages modern C++20 features with CMake build system and
        comprehensive testing suite for maintainable code.
      </>
    ),
  },
];

function Feature({title, emoji, description}: FeatureItem) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <span style={{fontSize: '4rem'}} role="img" aria-label={title}>
          {emoji}
        </span>
      </div>
      <div className="text--center padding-horiz--md">
        <Heading as="h3">{title}</Heading>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures(): ReactNode {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
